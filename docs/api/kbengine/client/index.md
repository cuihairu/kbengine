# Client Plugins

KBEngine is an open source game server engine that does not include the
complete client part, but we provide multiple client plug-ins that can quickly
interface with the server. Client plug-ins handle communication with
the server, registering accounts, login interface, provide an Entity base class, and
so on. The plug-in layer and the client UI layer interface with each other to
form a complete game client.
The plug-in provides the UI layer updated data for display received from
the server (eg, HP changes, the entities entering the clients View),
and the UI layer provides the plug-in layer with updated data to send back to the
server based on the clients input (e.g., the position and orientation of the character).
KBEngine implements different client plug-ins under different front-end
technologies, but each plug-in follows the methods and attributes listed
in this section of the API documentation. All plug-ins follow the same implementation standard
outlined here in the "Client Plugins" section of the documentation.
