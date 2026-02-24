![task_01kj7tx470fdste5ftm0erc6qf_1771937135_img_0](https://github.com/user-attachments/assets/0d95c3cc-cf50-4788-ac24-bfee373af6e4)


# cproxy

`cproxy` is a **lightweight HTTP reverse proxy written in C**, designed to be easy to understand, simple to install, and modular enough to extend.

The goal of this project is to provide a solid base for handling incoming HTTP requests, routing them to internal services (backends), and adding useful features such as logging, routing, and runtime statistics.

---

## Why this project

This project was built to:

- understand how a reverse proxy works at a low level
- have a lightweight and fully controllable component
- build a clean, open-source base that is easy to read and improve
- learn networking, HTTP parsing, and socket handling in C

In short: it is a learning-oriented project **with a real-world structure**, useful as a base for more advanced features.

---

## What it does (in short)

`cproxy`:

- listens on a port (server socket)
- accepts client connections
- reads the HTTP request
- decides where to route it
- forwards the request to the correct backend
- receives the response
- sends it back to the client
- writes logs and updates statistics

---
