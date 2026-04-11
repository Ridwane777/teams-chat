# MyTeams - NWP

Projet reseau de type client/serveur inspire de Teams, realise en C++20.

Le depot contient:
- un serveur TCP (`myteams_server`)
- un client console (`myteams_client`)
- une specification de protocole MTAP dans [RFC-0001-MTAP.md](RFC-0001-MTAP.md)

## Objectif

Implementer une application collaborative avec:
- gestion des utilisateurs
- equipes (teams)
- channels
- threads
- commentaires
- messagerie directe

La communication suit le protocole MTAP (MyTeams Application Protocol), decrit dans [RFC-0001-MTAP.md](RFC-0001-MTAP.md).

## Prerequis

- Linux
- `clang++` avec support C++20
    ```
    sudo apt update && sudo apt upgrade
    sudo apt install clang
    ```
- `make`
    ```
    sudo apt update
    sudo apt install make
    ```
    or
    ```
    sudo apt install build-essential
    ```
- bibliotheques reseau POSIX (`socket`, `poll`, etc.)

## Compilation

Depuis la racine du projet:

```bash
make
```

Cibles utiles:

```bash
make server   # build uniquement le serveur
make client   # build uniquement le client
make clean    # supprime objets temporaires
make fclean   # clean + binaires
make re       # rebuild complet
```

## Execution

### 1. Lancer le serveur

```bash
./myteams_server <port>
```

Exemple:

```bash
./myteams_server 4242
```

### 2. Lancer le client

Dans un autre terminal:

```bash
./myteams_client <server_ip> <port>
```

Exemple:

```bash
./myteams_client 127.0.0.1 4242
```

Le client lit les commandes en mode interactif (`>`).
Tapez `exit` pour quitter le client.

## Structure du projet

```text
include/
  client/client.hpp         # interface client TCP
  server/server.hpp         # interface serveur TCP
  server/ServerState.hpp    # etat metier (users/teams/channels/...)
  Entities.hpp              # modeles (User, Team, Channel, ...)
  Types.hpp                 # constantes et types communs

src/
  client/client.cpp         # point d'entree client
  server/server.cpp         # point d'entree serveur
  server/ServerState.cpp    # logique CRUD en memoire
  Entities.cpp              # constructeurs des entites

file/files/
  Protocol.hpp/.cpp         # helpers de parsing/serialisation MTAP

RFC-0001-MTAP.md            # specification protocolaire
Makefile                    # build
```

## Etat actuel

Le projet contient deja:
- la base reseau TCP (socket, bind, listen, accept, poll)
- une boucle client interactive
- des structures metier completes (`User`, `Team`, `Channel`, `Thread`, `Comment`, `Message`)
- un `ServerState` en memoire pour manipuler ces entites
- des utilitaires de protocole MTAP (encode/decode/parse/serialise)

Points encore en cours:
- branchement complet entre la boucle serveur et la logique metier
- gestion de toutes les commandes MTAP cote serveur
- gestion complete des evenements asynchrones
- persistance disque integree au binaire final

## Protocole MTAP

Le protocole est textuel et oriente lignes:

```text
<OPCODE> <key=value ...>\n
```

Exemples d'opcodes client -> serveur:
- `LOG` (login)
- `OUT` (logout)
- `ULS` (liste des utilisateurs)
- `TCR` (creation team)
- `LST`, `INF`, `USE` (navigation/context)

Reponses serveur:
- `OK`
- `ERR`
- `EVT` (evenements serveur push)

Pour le detail complet (champs, erreurs, limites de taille), voir `RFC-0001-MTAP.md`.

## Notes

- Le projet est en phase de developpement: certaines commandes peuvent etre non implementees selon l'etat courant du code.
- Le serveur et le client sont prevus pour fonctionner sur une connexion TCP persistante.
