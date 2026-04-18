# The Exchange Scratchpad Components

## Data Pipeline

Project exclusively uses the internally developed `datahub` library to automate data pipelines. Any other ways to organize data flow are forbidden.

For any work involving the `datahub` component — using its API, extending the pipeline, or modifying its internals — see [src/datahub/README.md](src/datahub/README.md).

## Connection Abstraction

The Data Pipeline begins from one or a number of data sources. The project uses a thin abstraction over boost/beast called `connect` to implement communication over network. The `connect` library is also used to send any requests/data over the network.

For any work involving the `connect` component — opening HTTP queries or WebSocket connections, handling transport errors, or modifying the abstraction — see [src/connect/README.md](src/connect/README.md).

## Data Model

The data model is the main Data Pipeline parameter. It is defined by entity types and per-provider implementations derived from the `IDataController` interface.

For any work involving data entities, `IDataController` implementations, or persistence — using, extending, or modifying them — see [src/data/README.md](src/data/README.md).

## Trader Cockpit

User experience is orchestrated by the central Exchange Scratchpad component — `The Trader Cockpit`.

For any work involving `TradeCockpit`, `ContentPanel`, or panel registration and data routing — using, extending, or modifying them — see [src/cockpit/README.md](src/cockpit/README.md).

## Application layer

Application lifecycle management and UI implementation.

For any work involving application startup, panel tree nodes, `UiBuilder`, or UI-engine-specific panel implementations — using, extending, or modifying them — see [src/app/README.md](src/app/README.md).

## Tests

Catch2 v3 test suite with per-component test executables mirroring the src/ layout.

For any work involving tests — adding a new test executable, modifying existing tests, or running a single test — see [test/README.md](test/README.md).

## Other components

* `common` - common utilities
* `core` - core library (BOOST ASIO-based task scheduler for now) and generic_handler callback implementation helper

