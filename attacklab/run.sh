#!/bin/bash
echo "Fixing permissions ..."
chmod +x ctarget
chmod +x rtarget

echo "Running Phase 1:"
./ctarget -q -i phase1/phase1.payload

echo "Running Phase 2:"
./ctarget -q -i phase2/phase2.payload

echo "Running Phase 3:"
./ctarget -q -i phase3/phase3.payload

echo "Running Phase 4:"
./rtarget -q -i phase4/phase4.payload

echo "Running Phase 5:"
./rtarget -q -i phase5/phase5.payload