# ProjetRio
## Description
Implementation du modele serveur proxy client en TCP avec code de detection et correction d'erreur (CRC)

Caractéristiques clés:
- [x] Serveur
- [x] Proxy
- [x] Client
- [x] Protocole TCP
- [x] Utilisation d'un **thread** par client du proxy
- [x] Gestion des erreurs et de la plupart des déconnexions
- [x] Gestion des clients multiples
- [x] CRC avec polynome `P(x) = x^8 + x^7 + x^4 + x^3 + 1` (8 bits)
- [x] Correction d'erreur si 1 bit d'erreur par tranche de 16 bits (char + code CRC) 
- [x] Detection d'erreur seulement, si 3 bits d'erreur par tranche de 16 bits

## Installation
Clonez le projet puis

```bash
cd ProjetRio
make
```

## Utilisation
### Serveur
```bash
bin/server <ip_server> <port_server> 
```
### Proxy
```bash
bin/proxy <ip_proxy> <port_proxy> <ip_server> <port_server>
```
### Client
```bash
bin/client <ip_proxy> <port_proxy>
```

### Exemple

![Demo](.github/assets/demo.gif)
