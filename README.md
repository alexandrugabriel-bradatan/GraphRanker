# GraphRanker

L'obiettivo del progetto è la gestione di una classifica tra grafi diretti pesa
pesati. La classifica tiene traccia dei `k` "migliori" grafi. Il programma
riceve in ingresso due parametri, una sola volta sulla prima riga separati da
spazio, il numero di nodi dei grafi `d` e la lunghezza della classifica `k`,
seguiti da una sequenza di comandi tra:

- `AggiungiGrafo [matrice di adiacenza]`
- `TopK`

`d`, `k` e il numero di grafi sono rappresentabili con numeri interi a 32 bit.

## Comandi

### AggiungiGrafo

`AggiungiGrafo` richiede di aggiungere un grafo a quelli considerati per stilare
la classifica. È seguito dalla matrice di adiacenza del grafo stesso, stampata
una riga per ogni rigo, con gli elementi separati da virgole.

I nodi del grafo sono da considerarsi etichettati logicamente con un indice
intero tra `0` e `d - 1`; il nodo in posizione `0` è quello la cui stella
uscente è descritta dalla prima riga della matrice.

I pesi degli archi del grafo elementi sono interi nell'intervallo `[0;2^32–1]`.

Esempio di utilizzo:

```txt
AggiungiGrafo
3,7,42
0,7,2
7,4,3
```

### TopK

Si consideri ogni grafo dall'inizio del programma fino al comando `TopK`
etichettato con un indice intero corrispondente al numero di grafi letti prima
di esso (partendo da 0). `TopK` richiede al programma di stampare gli indici
interi dei `k` grafi aventi i `k` valori più piccoli della somma dei cammini più
brevi tra il nodo 0 e tutti gli altri nodi del grafo.  Se ci sono più grafi con
lo stesso valore di metrica, si dà la precedenza ai primi arrivati. Le distanze
dei nodi non raggiungibili dal nodo 0 sono considerate nulle. I `k` indici sono
stampati su un unico rigo separati da uno spazio in un qualunque ordine.
