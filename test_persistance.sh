#!/bin/bash

PORT=4243

# Start from an empty state
rm -f myteams.dat

echo "--------------------------------------------------------"
echo "=> Starting a temporary server (port $PORT)..."
./myteams_server $PORT > /dev/null 2>&1 &
SERVER_PID=$!
sleep 1

echo "=> Test 1: creating user 'alice' and team 'AlphaTeam'"
# Send MTAP commands
(
  echo "LOG name=alice"
  sleep 0.2
  echo "TCR name=AlphaTeam description=Our%20first%20team"
  sleep 0.2
  echo "OUT"
  sleep 0.2
) | nc -q 1 127.0.0.1 $PORT > output_test1.txt
cat output_test1.txt

sleep 1

echo "=> Stopping the server with SIGINT (simulated CTRL+C) -> saving state"
kill -SIGINT $SERVER_PID
wait $SERVER_PID

echo "--------------------------------------------------------"

echo "=> Restarting the server to test reloading..."
./myteams_server $PORT > /dev/null 2>&1 &
SERVER_PID=$!
sleep 1

echo "=> Test 2: logging in as 'bob' and listing (LST) to check AlphaTeam was persisted..."
# Envoi de LOG et LST
(
  echo "LOG name=bob"
  sleep 0.2
  echo "LST"
  sleep 0.2
  echo "OUT"
  sleep 0.2
) | nc -q 1 127.0.0.1 $PORT > output_test2.txt
cat output_test2.txt

sleep 1

echo "=> Final server shutdown..."
kill -SIGINT $SERVER_PID
wait $SERVER_PID

if grep -q "AlphaTeam" output_test2.txt; then
  echo "=> PASS: AlphaTeam survived the restart"
  rm -f output_test1.txt output_test2.txt
else
  echo "=> FAIL: AlphaTeam was not reloaded"
  exit 1
fi
