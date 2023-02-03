# AssetDatabase
### A client and server program suite to facilitate supply item ordering
---

The software is a niche solution to a distinct problem, and an example of possible other solutions.

The Server is a simple interface for an SQLite database. It, by default, operates on port 9864 and listens for the incoming supply requests. The server initiates a small handshake with the client to verify connection and recieves request, retrieves from available quantity in database. It then returns the request to the client, corrected for available onhands.

The client has a light interface which allows for inventory level queries and aggregated order requests. 
