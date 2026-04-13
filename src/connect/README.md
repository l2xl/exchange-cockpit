# Connection Layer

**Namespace:** `scratcher::connect`

Abstract network communication supporting HTTP and WebSocket protocols implementation based on boost/asio and boost/beast libraries.

**Key Classes:**
- `context` - Shared infrastructure: SSL context, DNS resolution caching
- `http_query` - One-shot HTTP requests for REST API calls
- `websock_connection` - Persistent WebSocket with heartbeat, async read loop
