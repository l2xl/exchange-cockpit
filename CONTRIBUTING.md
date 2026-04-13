# The Exchange Scratchpad Components

## Data Pipeline `@src/datahub/README.md`

Project exclusively uses the internally developed `datahub` library to automate data pipelines. Any other ways to organize data flow are forbidden.

## Data Model `@src/data/README.md`

Data model is managed by the data entity types and per data provider implementations derived from `IDataController` interface.

## Data Sources `@src/connect/README.md`

Project uses thin abstraction over boost/beast library to implement communication over network.

## Trader Cockpit `@src/cockpit/README.md`

User Experience management is implemented by the central Exchange Scratchpad component - `The Trader Cockpit`.

## Application layer `@src/app/README.md`

Application lifecycle management and UI implementation

## Tests `@test/README.md`

Catch2 v3 test suite with per-component test executables mirroring the src/ layout.

## Other components

* `common` - common utilities
* `core` - core library (BOOST ASIO-based task scheduler for now)

# Build instructions `@BUILD.md`

# Legacy Project Structure (deprecated, keep for reference)

### Widget Layer (`src/widget/`)

Qt-based visualization components.

**Key Classes:**
- `ContentFrameWidget` - universal container for dynamic panels
- `DataScratchWidget` - Main QWidget canvas with coordinate transformation
- `Scratcher` - Abstract renderer base class
- `QuoteScratcher` - Renders candlesticks/trades
- `TimeRuler`, `PriceRuler` - Axis renderers
- `PriceIndicator` - Current price line

**Observer Pattern:** `m_data_view_listeners` for view change notifications

### Application Layer (`src/app/`)

Top-level UI orchestration.

**Key Classes:**
- `TradeCockpitWindow` - Main QMainWindow, manages multiple market panels
- `MarketViewController` - Per-symbol controller binding IDataController to DataScratchWidget
- `ViewController` - Abstract base with `Update()` method

**Current Integration Points:**
- `MarketViewController` holds reference to `IDataController` (ByBitDataManager)
- Registers handlers with data controller for trade/orderbook updates
- Manages view rectangle synchronization between widget and data
