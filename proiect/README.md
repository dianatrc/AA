# Tema - Editor de Text
**Tararache Diana Cristina 322CB**

Aceasta tema implementeaza un `Piece Table` care permite operatii de editare mult mai rapide, evitand copierea masiva a datelor.

# Structuri
Se va implementa o **lista dublu inlantuita de structuri**.
- structura `Piece`:
    - `buffer_type` - indica daca segmentul provine din `ORIGINAL_BUFFER` sau din `ADD_BUFFER`.
    - `start` - indexul de unde incepe textul in buffer-ul respectiv.
    - `length` - lungimea segmentului respectiv.
    - `next, prev` - pointeri catre segmentul urmator, respectiv segmentul anterior.
- structura `TextEditor`:
    - `original_buffer` - buffer imutabil cu textul initial.
    - `add_buffer` - buffer unde se adauga tot textul nou scris.
    - `head, tail` - capetele listei de piese.
    - `global_cursor` - pozitia curenta unde untilizatorul se afla.
    - `total_length` - lungimea totala a textului construit.

# Implementarea operatiilor principale
- functia **create_table**: initializeaza structura TextEditor. Aceasta aloca memorie pentru editor, incarca continutul initial (din fisier sau din string-ul text), configureaza buffer-ul de adaugare si creeaza prima piesa daca exista text initial.
- functia **advance_cursor**: muta cursorul global in text, asigurandu-se ca acesta ramane mereu in limite valide (intre 0 si lungimea totala).
- functia **show_global_cursor**: returneaza valoarea curenta a indexului la care se afla cursorul.
- functia **show_total_len**: returneaza lungimea totala a textului actual (suma lungimilor tuturor pieselor din lista).
- functia **add_text**: insereaza la pozitia curenta a cursorului. Logica gestioneaza trei cazuri:
    - la capete - adaugarea unei piese noi la inceputul sau la sfarsitul unei piese existente, sau in mijlocul unei piese - piesa originala este rupta in doua, iar textul nou este inserat intre cele doua parti rezultate.
- functia **delete_text**: sterge un numar specificat de caractere aflate inaintea cursorului. Functia calculeaza intersectia dintre zona de sters si piesele existente, realizand: eliminarea completa a pieselor (care cad integral in zona de stergere), scurtarea pieselor (pieselor afectate partial) sau split (daca stergerea are loc strict in interiorul unei singure piese).
- functia **extract_current_text**: reconstruieste textul complet intr-un singur string. Parcurge lista de piese de la head la tail si copiaza datele din buffer-ul corespunzator (Original sau Add).
- functia **save_current_text**: salveaza starea curenta a editorului. Poate exporta text-ul intr-un fisier (daca filename este diferit de NULL) folosind functia auxiliara **write_pieces**. Totodata, poate returna textul extras sau pozitia cursorului prin pointerii furnizati. 

# Implementarea functiilor auxiliare
- functia **read_file**: deschide un fisier in mod binar, ii determina dimensiunea folosind fseek si ftell, si ii citeste continutul intr-un buffer alocat dinamic.
- functia **create_piece**: aloca memorie pentru o structura de tip Piece si ii initializeaza campurile.
- functia **insert_piece**: gestioneaza legaturile din lista dublu inlantuita pentru a insera un nod nou dupa o piesa data sau la inceputul listei (daca piesa de referinta este NULL).
- functia **remove_piece**: extrage o piesa din lista, actualizand legaturile nodurilor vecine si elibereaza memoria ocupata de acel nod.
- functia **expand_buffer**: verifica daca exista spatiu suficient in add_buffer pentru noul text. Daca nu, dubleaza capacitatea bufferului folosind realloc pana cand cerinta este indeplinita.
- functia **map_index_to_piece**: traduce index-ul logic (pozitia in textul final) in locatie exacta. Identifica piesa care contine acel index, offset-ul in interiorul acelei piese si piesa precedenta pentru operatii de inserare. 
- functia **write_pieces**: parcurge lista de piese si scrie continutul fiecareia direct in fisierul specificat, accesand buffer-ul corect (Original sau Add) in functie de tipul piesei.

# Complexitatiile operatiilor
Descrierea eficientei operatiilor implementate in raport cu numarul total de caractere (N) si numarul total de piese (K).
- Operatia `add_text` are complexitatea `O(K)`. Trebuie sa se parcurga lista pentru a gasi piesa de la cursor (folosind map_index_to_piece). Inserarea propriu-zisa implica doar modificarea pointerilor nodurilor, fara a muta restul textului in memorie.
- Operatie `delete_text` are complexitatea `O(K)`. Similar cu inserarea, se parcurg piesele pentru a identifica zona de sters. Operatie modificare doar variabilele start/length sau elimina noduri.
- Operatia `advance_cursor` are complexitatea `O(1)`. Implica doar o operatie aritmetica de actualizare a variabilei global_cursor si verificarea limitelor.
- Operatia `map_index_to_piece` are complexitatea `O(K)`. Worst-case, se parcurge intreaga lista dublu inlantuita. 
- Operatia `extract_current_text` are complexitatea `O(N)`. Reconstructia textului necesita parcurgerea fiecarui segment si copierea fiecarui caracter in noul string alocat.
- Operatia `save_current_text` are complexitatea `O(N)`. Salvarea necesita parcurgerea tuturor datelor din buffere pentru a scrie continutul final in fisierul output.