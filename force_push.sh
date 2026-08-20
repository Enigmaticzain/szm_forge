#!/bin/bash
for i in {1..30}; do
    rm -f .git/index.lock
    git add -A
    if [ $? -eq 0 ]; then
        echo "Git add succeeded"
        break
    fi
    echo "Retrying git add..."
    sleep 2
done

for i in {1..30}; do
    rm -f .git/index.lock
    git commit -m "chore: push all new changes made by 42 plans and the latest app"
    if [ $? -eq 0 ]; then
        echo "Git commit succeeded"
        break
    fi
    echo "Retrying git commit..."
    sleep 2
done

git push
