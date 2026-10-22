#!/usr/bin/env python3
"""
Deadlock Simulation System - Local GUI Server & Launcher
OS Project-Based Learning (PBL) - Team 8
School of Engineering, St Aloysius (Deemed to be University)
"""

import http.server
import socketserver
import webbrowser
import os
import sys

PORT = 8088

class CustomHTTPHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        # Enable CORS and disable caching during dev/eval
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate')
        super().end_headers()

def main():
    gui_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(gui_dir)

    # Allow port reuse
    socketserver.TCPServer.allow_reuse_address = True

    try:
        with socketserver.TCPServer(("", PORT), CustomHTTPHandler) as httpd:
            url = f"http://localhost:{PORT}/index.html"
            print("=" * 60)
            print("  DEADLOCK SIMULATION SYSTEM - INTERACTIVE GUI CONSOLE")
            print("  Team 8 | St Aloysius (Deemed to be University)")
            print("=" * 60)
            print(f"  Serving GUI locally at: {url}")
            print("  Press Ctrl+C to terminate the server.\n")

            # Automatically launch the user's default browser
            try:
                webbrowser.open(url)
            except Exception as e:
                print(f"  Note: Please open {url} manually in your browser ({e}).")

            httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n[Shutdown] Stopping GUI server. Goodbye!")
        sys.exit(0)
    except OSError as e:
        print(f"\n[Error] Could not bind to port {PORT}: {e}")
        print(f"You can also open '{os.path.join(gui_dir, 'index.html')}' directly in your browser.")

if __name__ == "__main__":
    main()
