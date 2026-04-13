# Data Model Design

**Namespace:** `scratcher` (common entities, OrderBook), `scratcher::data` (common entity types), `scratcher::bybit` (ByBit-specific entities and data manager)

## Common principles

All the mutable data are managed by entity types declared under `entities` subfolder or `<data_provider>/entities` for provider specific where the <data-provider> examples are bybit, okx, binance, etc...

The data lifecycle is managed exclusively by `datahub` library facilities.

The data pipeline management implemented by the `IDataController` subclasses on a per data provider basis.

Most of the data are persisted using data_model template, parameterized by the data entity type (currently uses SQLite as the data store)

Some data, like the order book, does not need persistence and managed by an independent model type (`OrderBook`) which satisfies the `data_sink` model concept and can be used to parameterize `data_sink<OrderBook>`.

All the data can be assigned to the two categories with different lifecycles:

1. Public data with origin owned by the data provider. Updates received from the data provider are automatically overwrite the locally persisted data.
   Here are:
   * Instrument list
   * Order book
   * Public trades

2. Private data with origin owned by the user. These data updates may be originated locally or received from the data provider. These data must be never overwrited and always versioned so every update creates a new data record with a new version.
   Here are:
   * Account balance
   * Order history
   * Position history
   * Private Trade history

## Data Providers

Disclaimer: the data entities type model is still in prototyping stage and most of the entity types are implemented particularly for ByBit data provider.

Mature data model type structure will contain all the entity type definitions defined commonly for all data providers.
Particular per provider definitions must be convertable to the common ones and used mostly for data deserialization when received from the data provider and then converted by the data pipeline into common types for persistence and local operation. 

### Bybit

* ByBit specific data entity definitions: ./bybit/entities
* ByBit data controller implementation: ./bybit/data_manager.hpp, ./bybit/data_manager.cpp

