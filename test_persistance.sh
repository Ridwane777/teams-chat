#!/bin/bash

PORT=4243

echo "--------------------------------------------------------"
echo "=> Démarrage temporaire du serveur (port $PORT)..."
./myteams_server $PORT > /dev/null 2>&1 &
SERVER_PID=$!
sleep 1

echo "=> Test 1 : Création de l'utilisateur 'alice' et de la team 'AlphaTeam'"
# Envoi des commandes MTAP
(
  echo "LOG name=alice"
  sleep 0.2
  echo "TCR name=AlphaTeam description=Notre%20premiere%20equipe"
  sleep 0.2
  echo "OUT"
  sleep 0.2
) | nc -q 1 127.0.0.1 $PORT > output_test1.txt
cat output_test1.txt

sleep 1

echo "=> Arrêt du serveur par SIGINT (CTRL+C simulé) -> Sauvegarde en cours"
kill -SIGINT $SERVER_PID
wait $SERVER_PID

echo "--------------------------------------------------------"

echo "=> Relance du serveur pour tester le rechargement..."
./myteams_server $PORT > /dev/null 2>&1 &
SERVER_PID=$!
sleep 1

echo "=> Test 2 : Connexion de 'bob', on liste (LST) pour voir si AlphaTeam a bien été stockée..."
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

echo "=> Arrêt final du serveur..."
kill -SIGINT $SERVER_PID
wait $SERVER_PID

echo "=> Fin du test !"
