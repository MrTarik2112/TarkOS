#!/bin/bash
# TarkOS Nova Auto-Commit Utility
# Usage: ./autocommit.sh [interval_in_seconds]

INTERVAL=${1:-60}

echo "TarkOS Nova Auto-Commit Active (Interval: ${INTERVAL}s)"
echo "Press Ctrl+C to stop."

while true; do
    git add .
    # Only commit if there are changes
    if ! git diff --cached --quiet; then
        TIMESTAMP=$(date +"%Y-%m-%d %H:%M:%S")
        git commit -m "Nova Auto-Save: $TIMESTAMP"
        echo "[$TIMESTAMP] Changes committed automatically."
    fi
    sleep $INTERVAL
done
