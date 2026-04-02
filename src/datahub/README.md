# Datahub Component

## Overview

Datahub is a composable data pipeline where each stage is optional. All stages share the same `data_acceptor` concept — a callable `(Range&&) -> void` — so any stage can be directly wired to the next, skipping intermediate layers when they are not needed.

Datahub is designed as generic data pipeline where every class made as mutch generic as possible to nit couple any data types, containers, etc. Event if the previous element in the chain may have some restricted types due to its nature, the next must not rely on these types and restrictions and provide as generic implementation as possible.  

## Full Pipeline

```
JSON source (HTTP / WebSocket)
    │
    ▼
data_dispatcher<Acceptors...>          — async SPSC queue + strand; tries each adapter in order
    │
    ▼
data_adapter<DataType, Handler>        — Glaze JSON → C++ struct; on failure falls through to next
    │  handler lambda
    ▼
data_sink<Model>                       — links persistence model to the feed layer
    └─ model->accept(cache)            — persists / merges (data_model or custom model)
    │
    │  handle_data(cache copy)         — fires feed acceptor (subscribers notified immediately)
    ▼
data_feed  (sorted_data_feed / snapshot_data_feed)
    │  optional data_condition filter
    ▼
data_subscription<Entity>             — leaf callbacks; snapshot delivered to new subscriber on attach
```

## Reduced Pipelines

All chainlinks expose `data_acceptor<Range>()` returning a callable. Any stage can be bypassed by wiring acceptors directly:

**No persistence — feed only:**
```
data_adapter → data_feed->data_acceptor<Range>()
```

**No feed — model only:**
```
data_adapter → data_sink<data_model>(model, [](cache&&){}, error_cb)
               └─ handle_data no-op; model persists
```

**No sink/model — adapter straight to custom handler:**
```
data_adapter → any callable(Range&&)
```

**No dispatcher — direct model feed (async, strand-posted):**
```
data_model::data_acceptor<Range, Container>()  — posts accept() on db_strand
```

## Component Details

### generic_handler<DATA, PARENT, DATA_CALLABLE, ERROR_CALLABLE, ARGS...> (`src/common/generic_handler.hpp`)
- CRTP mixin that implements `handle_data(DATA)` and `handle_error(exception_ptr)` as virtual overrides via stored callables
- Inherits from `PARENT` (the abstract base being implemented), forwarding `ARGS...` to its constructor
- `handle_data` wraps the callable in try/catch, routing exceptions to `handle_error`
- Used by `data_sink::create()` to produce a concrete subclass without a separate derived class per use case

### data_dispatcher<Acceptor...> (`src/datahub/data_sink.hpp`)
- `boost::lockfree::spsc_queue<std::string>` (1024-entry)
- Serialized via `boost::asio::strand`
- `operator()(std::string)` — pushes to queue, posts async dispatch
- Tries each acceptor in sequence via fold: `(try_accept<I>(data) || ...)` — first to return `true` wins

### data_adapter<DataType, Handler> (`src/datahub/data_sink.hpp`)
- Deserializes JSON string via `glz::read<opts{.error_on_unknown_keys = false}>`
- On success: calls handler, returns `true`
- On failure: returns `false`, dispatcher falls through to next adapter
- Factory: `make_data_adapter<DataType>(handler)`

### data_sink<Model> (`src/datahub/data_sink.hpp`)
- `accept<Range>(data)`:
  1. `handle_data(cache_type(cache))` — virtual; implemented by `generic_handler`, typically fires the feed acceptor
  2. `m_model->accept(std::move(cache))` — delegates to model
- `data_acceptor<Range>()` — returns lambda over `weak_ptr<data_sink>` calling `accept<Range>`
- `create(model, data_handler, error_handler)` — factory via `generic_handler` mixin
- Factory: `make_data_sink(model, data_handler, error_handler)`

**Model concept** (implicit): any type with `entity_type` alias, `cache_type` alias, and `accept(Range&&)` method.

### data_model<Entity, auto PrimaryKey> (`src/datahub/data_model.hpp`)
- SQLite DAO with Glaze reflection for automatic schema generation; RAII table creation
- `accept<Range>(entities)` — `insert_or_replace` per entity; returns only actually-changed rows (dedup via `sqlite3_changes()`)
- `data_acceptor<Range, Container>()` — async variant: posts `accept()` on `m_db_strand` via `weak_ptr`
- `query(condition, args...)` — SELECT with optional WHERE
- `strand_type = boost::asio::strand<boost::asio::any_io_executor>` — public alias; one strand shared across all models backed by the same DB
- Factory: `data_model<E, PK>::create(db, strand, table_suffix)`

### data_feed concept (`src/datahub/data_feed.hpp`)
- Abstract data feed concept 

#### sorted_data_feed<Entity, SortField, KeyField> 
- In-memory cache sorted ascending by `SortField`, deduplicated by `KeyField`
- `data_acceptor<Range>()` — filters by optional condition, skips known keys, inserts sorted, notifies with `update_kind::increment` or `update_kind::snapshot`
- `subscribe(weak_ptr<subscription_type>)` — fires current cache as snapshot to new subscriber if non-empty

#### snapshot_data_feed<Entity>
- In-memory cache that replaces entire state on each update
- `data_acceptor<Range>()` — clears, filters by optional condition, rebuilds, notifies with `update_kind::snapshot`
- `subscribe(weak_ptr<subscription_type>)` — fires current cache as snapshot to new subscriber if non-empty

#### db_data_feed<Entity>
- DB feed which translates query with condition into resulting range with DB cursor
TODO

Both feed types accept an optional `shared_ptr<data_condition<Entity>>` at creation or via `set_condition()`.

### data_condition<Entity> (`src/datahub/data_condition.hpp`)
- Composite AND filter dual-purpose: in-memory predicate and SQL `QueryCondition` generation
- `field_predicate<Entity, Field, Op>` — compile-time field pointer + operator (`QueryOperator` enum), runtime value
- `data_condition(Predicates...)` — variadic constructor composing predicates
- `matches(const Entity&)` — all predicates must pass
- `to_query_condition()` — produces SQL WHERE clause
- Static factory methods: `equal<Field>(v)`, `not_equal<Field>(v)`, `less<Field>(v)`, `less_or_equal<Field>(v)`, `greater<Field>(v)`, `greater_or_equal<Field>(v)`

### data_subscription<Entity, Container> (`src/datahub/data_subscription.hpp`)
- Abstract leaf: `handle_update(update_kind, const Container&) = 0`
- Factory: `make_data_subscription<Entity>(callable)` → `shared_ptr<data_subscription<Entity>>`; callable: `(update_kind, const deque<Entity>&) -> void`

### update_kind (`src/datahub/data_update.hpp`)
- `enum class update_kind { snapshot, increment }`

## Key Design Rules

- **Weak ptr safety**: all async callbacks capture `weak_ptr` to avoid circular refs and handle object lifetime
- **Snapshot-on-subscribe**: both feed types immediately deliver current state to a newly attached subscriber
- **Dedup at model layer only**: `data_model::accept()` filters duplicate rows; feeds always receive raw incoming data
- **One strand per DB**: all `data_model` instances backed by the same database share one `strand_type`
- **Thread safety boundary**: `data_dispatcher` serializes JSON dispatch; `data_model` serializes DB access via strand; feed mutation happens on whichever thread calls the acceptor (usually data_dispatcher)
