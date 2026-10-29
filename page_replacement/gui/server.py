#!/usr/bin/env python3
"""Serve the page-replacement simulator locally and open it in a browser."""

from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
import webbrowser


HOST = "127.0.0.1"
PORT = 8090
GUI_DIR = Path(__file__).resolve().parent


def main():
    handler = partial(SimpleHTTPRequestHandler, directory=str(GUI_DIR))
    try:
        with ThreadingHTTPServer((HOST, PORT), handler) as server:
            url = f"http://{HOST}:{PORT}/index.html"
            print(f"Page Replacement Simulator: {url}")
            print("Press Ctrl+C to stop the local server.")
            try:
                webbrowser.open(url)
            except Exception:
                print(f"Open this address in a browser: {url}")
            server.serve_forever()
    except KeyboardInterrupt:
        print("\nServer stopped.")
    except OSError as exc:
        print(f"Could not start the local server on port {PORT}: {exc}")


if __name__ == "__main__":
    main()
