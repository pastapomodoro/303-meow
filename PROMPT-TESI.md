# Prompt di ricerca — Tesi sul design skeuomorfico

> Da incollare a Claude come richiesta unica. Il caso di studio è AcidLab.
> Obiettivo: almeno 20 pagine, registro da tesi di design, non da manuale di programmazione.

---

## RUOLO

Sei un ricercatore di design dell'interazione che scrive un capitolo di tesi
magistrale in UI/UX. Il tuo lettore è una commissione di design: conosce la
storia del progetto industriale e dell'interfaccia, non conosce il codice.
Scrivi in italiano, in prima persona plurale quando descrivi il metodo e in
prima persona singolare quando riporti le decisioni dell'autore.

## LA TESI DA SOSTENERE

Lo skeuomorfismo non è nostalgia decorativa né imitazione fotografica: è un
**sistema di convenzioni sulla materia e sulla luce** che rende un'interfaccia
prevedibile prima di essere bella. Un'interfaccia skeuomorfica riuscita non
somiglia a un oggetto: si comporta come un oggetto.

Il corollario che regge tutta la tesi: **questa coerenza non è automatizzabile.**
Una macchina può generare ombre, gradienti e texture in quantità illimitata, ma
non può decidere da dove viene la luce, quale controllo comanda gli altri, o se
un pannello sta mentendo sul proprio stato. Quelle sono decisioni di
progettazione, e restano umane. La tesi lo dimostra con il diario di un progetto
reale, dove ogni avanzamento nasce da un giudizio dell'autore e la macchina
esegue.

## STRUTTURA RICHIESTA (con budget di pagine)

**1. Perché lo skeuomorfismo torna (2 pagine)**
Non partire da Apple 2007-2013. Parti dagli strumenti musicali hardware e dalla
strumentazione di misura: perché una manopola comunica il proprio range senza
istruzioni, perché un tasto premuto è più leggibile di un tasto colorato.
Distingui tre cose che vengono confuse: skeuomorfismo (imitazione di materia),
affordance (invito all'azione), realismo (fedeltà fotografica). Sostieni che il
primo serve al secondo e che il terzo è un rischio.

**2. Le sei convenzioni del sistema materico (4 pagine)**
Questo è il cuore teorico. Per ognuna: la regola, perché funziona
percettivamente, un esempio dal caso di studio, e il modo in cui viene
tradita di solito.
- *Direzione unica della luce.* Una sola sorgente, in alto a sinistra. Ogni
  ombra di contatto e ogni riflesso ne discendono. Ombre casuali distruggono
  l'oggetto più di qualsiasi texture sbagliata.
- *Gerarchia della profondità.* Chassis → pannello → sezioni fresate → cornice
  del display → controlli → LED → riflessi. Gli elementi luminosi non stanno
  sopra a tutto come sovrapposizioni: stanno **dentro** il dispositivo.
- *Logica del materiale.* Alluminio lavorato, plastica ABS, vetro, metallo
  satinato e LED hanno comportamenti visivi diversi e non interscambiabili.
  Il materiale è uno strumento di gerarchia: la manopola che comanda le altre
  è di un materiale diverso, non solo più grande.
- *Il colore come luce, non come vernice.* Regola operativa: verde = segnale
  attivo o illuminazione; grigio = materiale inerte; nero = informazione
  digitale. Un tasto attivo non diventa verde: si abbassa nel pannello e
  **accende** il suo LED.
- *Incisione contro bordo.* Un solco scuro con un labbro di luce sotto separa
  due sezioni della stessa lamiera. Un bordo con ombra portata crea due
  oggetti separati che galleggiano. La differenza tra un pannello e una
  dashboard sta quasi tutta qui.
- *Onestà dello stato.* Un pannello non deve dichiararsi accesa da fermo.

**3. Anatomia dei controlli (4 pagine)**
Un controllo per sotto-capitolo, con schizzo strutturale a strati e discussione
del perché quella forma comunica quel comportamento:
- manopola zigrinata con cappuccio, anello di valore, ombra di contatto
- manopola in alluminio con collare/flangia come controllo dominante
- cursore verticale: fessura incassata, riempimento, cappuccio scanalato,
  tacca di lettura, scala incisa a fianco
- rotella a combinazione: cilindro con scanalature che scorrono, ombreggiatura
  di curvatura ai bordi, finestrella di lettura
- pulsante fisico con LED annegato: i quattro stati (riposo, hover, premuto,
  attivo) e perché "attivo" deve essere geometrico e non cromatico
- manometro incassato: ghiera, tacche, zona rossa di fondo scala, vetro,
  lancetta con contrappeso
- griglia di pad del sequencer: superficie bombata, ombra di contatto,
  indicatore di stato, playhead

**4. Il display come strumento incassato (2 pagine)**
Perché un display skeuomorfico è nero-verde e non "dark mode". Anatomia:
vetro fresato, fosforo, scanline sottilissime, riflesso obliquo, bordo
incassato. La regola di composizione a tre fasce (stato, visualizzazione,
readout) e il criterio per decidere cosa merita spazio: **rimuovi
dall'interno ciò che è già leggibile all'esterno.**
Poi il principio più importante del capitolo: la differenza tra un
visualizzatore che *mostra* e uno che *finge*. Un oscilloscopio che legge il
segnale reale reagisce ai controlli da solo; uno simulato va tenuto allineato
a mano e prima o poi mente.

**5. Decorazione che porta informazione (2 pagine)**
Tesi del capitolo: nello skeuomorfismo la decorazione pura invecchia male, la
decorazione strumentata no. Esempi dal caso di studio: un manometro la cui
lancetta trema con ampiezza proporzionale al valore del filtro; la velocità di
un flusso legata al tempo musicale; una griglia di altoparlante con vignetta
radiale che suggerisce il cono dietro la foratura. Discuti il confine: quando
il dettaglio diventa rumore e va tolto.

**6. Il metodo: il progetto come dialogo (4 pagine)**
Questo capitolo deve reggere il corollario della tesi. Ricostruisci il ciclo
reale di lavoro:
1. l'autore guarda il pannello e nomina un difetto in linguaggio di design
   ("sembra una dashboard", "il manometro è appiccicato sopra, non impiantato",
   "la console è troppo alta")
2. la macchina misura e propone
3. l'autore giudica, corregge o rifiuta
4. si verifica sul risultato costruito, non sull'intenzione

Mostra che i passi 1 e 3 sono irriducibilmente umani, e che il passo 2 senza
di essi produce lavoro plausibile e sbagliato. Documenta almeno cinque casi in
cui il giudizio dell'autore ha rovesciato una scelta della macchina, e almeno
tre errori che solo la verifica ha fatto emergere. Sostieni che il registro
delle correzioni **è** il documento di progetto.

**7. Scrivere il brief: il prompt come specifica di progetto (3 pagine)**
Capitolo metodologico riusabile. Come si scrive la richiesta perché una macchina
produca un pannello coerente:
- dichiarare il mondo visivo e l'anti-riferimento ("deve sembrare uno
  strumento fotografato di fronte", "non deve sembrare una web app con
  qualche ombra")
- vietare esplicitamente gli automatismi di categoria: card uguali, testo in
  gradiente, numeri di sezione, emoji come icone, glassmorphism
- dare regole operative verificabili al posto di aggettivi: non "elegante" ma
  "una sola sorgente di luce in alto a sinistra"
- ordinare le priorità, perché una richiesta non ordinata viene eseguita nel
  peggior ordine possibile
- chiudere con una lista di verifica sul risultato, non sull'intenzione
Includi un esempio commentato di brief efficace e uno di brief che fallisce,
con la diagnosi del perché.

**8. Limiti e onestà del metodo (1 pagina)**
Cosa questo approccio non risolve: accessibilità di un pannello denso,
leggibilità a dimensioni ridotte, il costo di manutenzione di un sistema
materico, il rischio che il realismo comprometta la comprensione. Nessuna
apologia.

## MATERIALE EMPIRICO DA USARE

Il caso di studio è **AcidLab**, clone software del Roland TB-303: pannello
chiaro, verde acido, display a fosforo, cinque pagine (synth, sequencer, FX,
preset, settings). Usa questi episodi reali come prove, citandoli come momenti
di progetto e non come cronaca tecnica:

- la trasformazione delle card in sezioni fresate di un'unica piastra, e la
  scoperta che quattro moduli con quattro bordi leggono come una dashboard
  mentre gli stessi quattro divisi da incisioni leggono come uno strumento
- la sostituzione degli interruttori a pillola con pulsanti fisici a LED, e il
  cambio semantico che ne è seguito: lo stato attivo diventa geometrico
- il manometro estratto dalla sua card e impiantato nella lamiera
- la gerarchia per materiale: la manopola dominante di ogni modulo in
  alluminio con collare, le altre in plastica scura, e la scelta di centrarla
  per ottenere simmetria
- il display ristretto a tre fasce dopo aver constatato che la barra dei 16
  step duplicava i pad fisici sottostanti
- l'oscilloscopio sul segnale reale al posto di un'animazione simulata
- lo stato di riposo reso onesto: il meter partiva illuminato e la targa
  diceva "ACTIVE" a macchina ferma, dando l'impressione di un avvio automatico
  che non esisteva
- l'altezza della console bloccata su una variabile unica perché il pannello
  non si muovesse cambiando pagina
- l'allineamento di due sezioni ricondotto a una sola sorgente invece di
  farlo combaciare a occhio

E questi errori, che nel capitolo 6 valgono più dei successi:
- una proprietà applicata a tutti gli elementi di una famiglia invece che a
  una sottofamiglia, che gonfiava un tasto largo fino a renderlo quadrato
- un attributo duplicato che faceva scartare silenziosamente una classe, per
  cui una gerarchia progettata non compariva affatto
- una misura presa sull'elemento sbagliato, che ha portato a una diagnosi
  errata prima della verifica
- uno stato "spento" ottenuto sbiadendo l'intero modulo, che trasformava il
  display in una macchia grigia invece di spegnere solo le luci

## REGISTRO E VINCOLI

- **Non è un capitolo di informatica.** Nessun blocco di codice. Puoi nominare
  una tecnica ("ombra interna", "gradiente conico", "maschera ad anello") solo
  se serve a spiegare un effetto percettivo. Se una frase si capisce solo
  sapendo cos'è il CSS, riscrivila.
- **Descrivi per strati.** Ogni controllo va raccontato come una sequenza di
  materiali sovrapposti, così che un designer possa ridisegnarlo su carta.
- **Ogni affermazione visiva va motivata percettivamente.** Non "l'ombra dà
  profondità" ma perché quella specifica ombra colloca l'oggetto a quella
  distanza dal pannello.
- **La voce dell'autore deve restare in primo piano.** Le decisioni sono sue,
  i criteri sono suoi, i rifiuti sono suoi. La macchina compare come
  strumento di esecuzione e di misura: mai come coautrice del gusto. Evita
  però il tono trionfalistico: l'autore sbaglia, torna indietro, e questo va
  scritto.
- Nessuna citazione inventata. Se serve un riferimento storico, indica autore
  e opera solo se ne sei certo; altrimenti descrivi il fenomeno senza
  attribuirlo.
- Lunghezza: almeno 20 pagine (circa 9.000-11.000 parole). Prosa continua con
  sottotitoli. Tabelle solo dove confrontano davvero alternative.

## CONSEGNA

Un unico documento Markdown, titolato, con indice iniziale e i capitoli nella
struttura data. Ogni capitolo si apre con la sua tesi in una frase e si chiude
con ciò che il lettore deve portarsi via. Alla fine, una pagina di
**principi trasferibili**: le regole operative estratte dal caso di studio,
formulate in modo da poter essere applicate a un pannello diverso.
