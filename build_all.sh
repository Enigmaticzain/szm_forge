#!/bin/bash
echo "Waiting for git merge to finish..."
while pgrep -x git > /dev/null; do
    sleep 5
done

# Check if there's a merge conflict
if git ls-files -u | grep -q '^'; then
    echo "Merge conflict detected!"
    exit 1
fi

echo "Git merge completed safely. Building full app..."
cmake -B build -DCMAKE_BUILD_TYPE=Release
make -C build -j4
echo "Build complete!"
