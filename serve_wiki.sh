#!/bin/bash
# Simple HTTP server to view the ReproVM wiki locally

PORT=${1:-8000}

echo "🚀 Starting ReproVM Wiki Server..."
echo "📖 Open your browser to: http://localhost:$PORT"
echo "🛑 Press Ctrl+C to stop"
echo ""

# Try different server options based on what's available
if command -v python3 &> /dev/null; then
    echo "Using Python 3 HTTP server..."
    python3 -m http.server $PORT
elif command -v python &> /dev/null; then
    echo "Using Python 2 HTTP server..."
    python -m SimpleHTTPServer $PORT
elif command -v node &> /dev/null && npm list -g http-server &> /dev/null; then
    echo "Using Node.js http-server..."
    npx http-server -p $PORT
elif command -v php &> /dev/null; then
    echo "Using PHP built-in server..."
    php -S localhost:$PORT
else
    echo "❌ Error: No suitable HTTP server found."
    echo "Please install one of: python3, python, node (with http-server), or php"
    exit 1
fi
