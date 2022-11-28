# Zadanie 1 — `stdout`
## Co studenciak musi wiedzieć
### man 3p stdin
```c
#include <stdio.h>

extern FILE *stderr, *stdin, *stdout;
```

`stream` to plik z powiązanym buforowaniem i jest zdefiniowany jako wskaźnik na typ `FILE`. Funkcja `fopen()` tworzy `stream` i zwraca wskaźnik. Normalnie otwarte są trzy `stream`y ze stałymi wskaźnikami w nagłówku `<stdio.h>` skojarzone z otwartymi plikami standardowymi.

Przy uruchomieniu programu, trzy pliki tworzone są domyślnie:
- `stdin` — wejście standardowe
- `stdout` — wyjście standardowe (zwykłe)
- `stderr` — wyjście standardowe dla błędów
Po otwarciu `stderr` nie jest w pełni buforowane, `stdin` i `stdout` są $\iff$ `stream` nie jest przypisany do urządzenia interaktywnego.

W `<unistd.h>` zdefiniowane są deskryptory przypisane plikom `stdin`, `stdout`, `stderr`:
- `STDIN_FILENO` — deskryptor `stdin`, równy 0
- `STDOUT_FILENO` — deskryptor `stdout`, równy 1
- `STDERR_FILENO` — deskryptor `stderr`, równy 2
`stderr` powinien być otwarty do zapisu i odczytu!

### man 3p printf
```c
#include <stdio.h>

int printf(const char *restrict format, ...);
```
Wypisuje sformatowany output. Bazowo robi: `fprintf(stdout, format, ...);`

### man stdlib.h
Biblioteka standardowa C. Definiuje *między innymi*:
- `EXIT_FAILURE` — wyjście z niepowodzeniem dla `exit()`, jakaś niezerowa wartość
- `EXIT_SUCCESS` — wyjście z powodzeniem dla `exit()`, 0
- `RAND_MAX` — maksymalna wartość zwrócona przez `rand()`, przynajmniej `32767` (`0x7FFF`)
- `NULL`
- typy `size_t`, `wchar_t`
- funkcje:
	- `int abs(int)`
	- `double atof(const char *)`
	- `int atoi(const char *)`
	- `long atol(const char *)`
	- `long long atoll(const char *)`
	- `void *calloc(size_t, size_t)`
	- `void exit(int)`
	- `void free(void *)`
	- `char *getenv(const char *)`
	- `void *malloc(size_t)`
	- `int rand(void)`
	- `void *realloc(void *, size_t)`
	- `int setenv(const char *, const char *, int)`
	- `void srand(unsigned)`
	- `double strtod(const char *restrict, char **restrict)`
	- `float strtof(const char *restrict, char **restrict)`
	- `long strtol(const char *restrict, char **restrict, int)`
	- `long double strtold(const char *restrict, char **restrict)`
	- `long long strtoll(const char *restrict, char **restrict, int)`
	- `unsigned long strtoul(const char *restrict, char **restrict, int)`
	- `unsigned long long strtoull(const char *restrict, char **restrict, int)`

### man make
`make` to narzędzie, które jest w stanie określić, które pliki potrzebują rekompilacji i ewentualnie ją wywołać.  Żeby użyć `make`, trzeba mieć `makefile`, który opisuje relacje między plikami w programie i definiuje komendy ich aktualizacji. Np. dla pliku `program.c` można zdefiniować komendę `gcc program.c -o program`, więc gdy `make` wykryje zmiany w pliku `program.c`, skompiluje go.

Aby zrekompilować wszystko wystarczy użyć komendy 
```bash
make
```
827349832 opcje są nie chce mi się xoxox

Cele opisane jako `PHONY` nie są powiązane z fizycznie istniejącymi plikami.

## Pytanka kontrolne 
1. Gdzie znaleźć dokumentację samego polecenia `man`?
```bash
man man
```

2. Czemu wpisanie `man printf` nie pomoże nam w zrozumieniu funkcji `printf`?
>Bo `man printf` wyświetli pierwszą stronę, jaką znajdzie, a pierwsze znajdzie się `printf(1)`, polecenie powłoki.

3. Skąd wiadomo jakie pliki nagłówkowe trzeba włączyć?
>Z `man`a, dla `printf` jest `stdio.h`, dla stałych `stdlib.h`.

4. Skompiluj program poleceniem „make prog1”, używasz w ten sposób domyślnego szablonu kompilacji programu GNU make. Uruchom program wynikowy. Czemu taki sposób kompilacji będzie dla nas nieprzydatny?
>Bo nie ma `-Wall`, kompilator nas nie ostrzega

Dany jest makefile:
```makefile
all: prog1
prog1: prog1.c	
	gcc -Wall -fsanitize=address,undefined -o prog1 prog1.c
.PHONY: clean all
clean:
	rm prog1
```

5. Jak za pomocą programu make i podanego Makefile usunąć stary plik wykonywalny?
```bash
make clean
```

6. Jak za pomocą programu make i podanego Makefile przeprowadzić kompilacje?
```bash
make [prog1]
```

7. Jak przekierować wyjście tego programu do pliku?
```bash
./prog1 > plik.txt
```

8. Jak teraz wyświetlić ten plik?
```bash
cat plik.txt
```

---
# Zadnie 2 — `stdin`, `stderr`
## Co studenciak musi wiedzieć
### man 3p fscanf
#### Skrót
```c
#include <stdio.h>

int fscanf(FILE *restrict stream, const char *restrict format, ...);
int scanf(const char *restrict format, ...);
int sscanf(const char *restrict s, const char *restrict format, ...);
```

#### Opis
Czyta ze `stream`a. `scanf()` to bazowo `fscanf(stdin, format, ...)`. `sscanf()` czyta ze stringa. Wszystkie te funkcje czytają bajty ze swojego wejścia i interpretują je zgodnie z `format`em, a następnie wpisują do kolejnych argumentów będących <u>wskaźnikami</u> na zmienne.  Gdy argumentów jest za mało dla podanego formatu, wynik jest niezdefiniowany. Gdy format się skończy, a wciąż są argumenty, to zostaną pobrane ale zignorowane.

Ten manpag ma dosłownie 23891320132890132890123890123089123089 linijek :(((
Czytając `%s`, `scanf` doda `'\0'` na koniec tego napisu.

#### Return
+ Jeśli wszystko zakończyło się sukcesem, funkcje te zwracają **liczbę udanych konwersji/dopasowań**.
+ Gdy już pierwsza konwersja się nie powiedzie — zwróci 0.  Jeśli input skończy się przed końcem pierwszej konwersji, bez błędu konwersji, to zwrócony zostanie `EOF`.
+ Jeśli w trakcie konwersji wystąpi błąd, bez niepowodzenia konwersji, zwrócony zostanie `EOF` a `errno` zostanie ustawione na odpowiedni błąd. 
+ Jeśli nastąpi błąd odczytu, ustawiony zostanie indykator błędu `stream`a. 

### man 3p perror
#### Skrót
```c
#include <stdio.h>

void perror(const char *s);
```

#### Opis
Przypisuje liczbowej zmiennej `errno` tekstową wiadomość o błędzie, która wypisana zostanie na `stderr` w następujący sposób:
- najpierw, jeśli `s != NULL` oraz `*s != '\0'`, to napis `s`, a po nim dwukropek i spacja
- potem tekst błędu a po nim `<newline>`

Tekstem o błędzie jest wartość `strerror()` dla argumentu `errno`. W przypadku błędu ustawione zostanie błąd w `stderr`, a błąd zostanie ustawiony w `errno`.

#### Return
Nic.

### man 3p fprintf
#### Skrót
```c
#include <stdio.h>

//int dprintf(int fildes, const char *restrict format, ...);
int fprintf(FILE *restrict stream, const char *restrict format, ...);
int printf(const char *restrict format, ...);
int snprintf(char *restrict s, size_t n, const char *restrict format, ...);
int sprintf(char *restrict s, const char *restrict format, ...);
```

Funkcja `fprintf()` wypisuje output do podanego `stream`a. 
Funkcja `printf()` wypisuje output na `stdout`. 
Funkcja `sprintf()` wypisuje output do stringa, a <u>po nim bajt</u> `'\0'`.  
Funkcja `snprintf()` to odpowiednik `sprintf()`, ale dodatkowo pobiera argument `n`, który mówi o wielkości bufora `s`. Jeśli `n == 0`, nic nie zostanie wpisane. W przeciwnym wypadku, znaki po `(n-1)`szym zostaną odrzucone, a na końcu zostanie wpisany bajt `'\0'`.

#### Return
+ W przypadku powodzenia `fprintf()` i `printf()` zwrócą liczbę wypisanych bajtów.
+ W przypadku powodzenia `sprintf()` zwraca liczbę bajtów wpisanych do `s`, nie licząc `'\0'`.
+ W przypadku powodzenia `snprintf()` zwróci liczbę bajtów, które byłyby wpisane do `s`, gdyby `n` było odpowiednio duże, nie licząc `'\0'`.

+ W przypadku błędu funkcje te zwrócą `-1` i ustawią `errno` na błąd.

## Pytanka kontrolne
Przykładowy kod `prog2.c`
```c
char name[22];
scanf("%21s", name);
if (strlen(name) > 20) ERR("Name too long");
printf("Hello %s\n", name);
```

1. Czemu w kodzie pojawia się 21 jako rozmiar maksymalny w formatowaniu `scanf` (`%21s`)``?
> Bo czytając 20 znaków, nie mamy możliwości sprawdzić, czy nie podano za dużo znaków. Czytając 21 znaków, jeżeli `name` ma długość 21 wiemy, że podano zbyt długą nazwę.

2. Czemu deklarujemy 22 jako rozmiar tablicy na ciąg znaków, skoro czytamy najwyżej 21 znaków?
> Bo napisy w C kończą się `'\0'`, scanf je dopisze

3. Jak można zmienić sposób wywołania tego programu tak, aby komunikat o ewentualnym błędzie wykonania nie pojawił się na ekranie?
```bash
./prog2 2>/dev/null
```

---
# Zadanie 3 — `stdin` cd.
## Co studenciak musi umieć
### man 3p fgets
#### Skrót
```c
#include <stdio.h>

char *fgets(char *restrict s, int n, FILE *restrict stream);
```

#### Opis
Funkcja `fgets()` czyta bajty ze `stream`a do tablicy `s`, dopóki nie zostanie przeczytane `n-1` bajtów, lub pojawi się znak `<newline>` i zostanie wpisany do `s`, lub dojdziemy do końca pliku. Po wczytanych bajtach zostanie wstawiony znak `'\0'`. Jeśli EOF nastąpi przed przeczytaniem czegokolwiek, `s` nie zostanie zmieniony.

#### Return
+ W przypadku sukcesu, `fgets()` zwróci `s`.
+ W przypadku, gdy `stream` jest w stanie EOF, `fgets()` zwróci `NULL`.
+ W przypadku błędu, `fgets()` zwróci `NULL` i ustawie `errno` na odpowiednią wartość.

### Skrrróty powłoki bash
- `CTRL + d` — zamknięcie strumienia, wprowadzenie go w stan EOF. Działa tylko po znaku nowej linii!!
- `CTRL + c` — wysyła `SIGINT` do procesu i pozwala zazwyczaj zakończyć cały program
- `CTRL + z` — zawiesza program (wysyła `SIGSTOP`), można potem poleceniem `jobs` zobaczyć listę takich "wiszących" programów i przywrócić wybrany do życia (pisząc `%N`, gdzie `N` to numer procesu)
- `CTRL + \` — wysyła `SIGQUIT`, kończy program i generuje zrzut pamięci
- `CTRL + s` — zamraża terminal, aby go odwiesić — `CTRL + q`

## Pytanka kontrolne
`prog3.c`:
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 20

int main(int argc, char **argv)
{
	char name[MAX_LINE + 2];
	while (fgets(name, MAX_LINE + 2, stdin) != NULL)
		printf("Hello %s", name);
	return EXIT_SUCCESS;
}
```
`Makefile`:
```makefile
all: prog1 prog2 prog3
prog1: prog1.c
	gcc -Wall -fsanitize=address,undefined -o prog1 prog1.c
prog2: prog2.c
	gcc -Wall -fsanitize=address,undefined -o prog2 prog2.c
prog3: prog3.c
	gcc -Wall -fsanitize=address,undefined -o prog3 prog3.c
.PHONY: clean all
clean:
	rm prog1 prog2 prog3
```

1. Jak dla powyższego `Makefile` skompilować tylko jeden cel
```bash
make prog3
```

2. Sprawdź, jak się zachowa dla ciągów 20 i 21 znakowych. Czemu akurat tak?
>Dla ciągu długości 20 śmiga :) Dla ciągu 21 wypisuje `Hello ABCDERFTHGJUIALKSOIMNHello`, bo `fgets` jest bezpieczną funkcją i nie pozwala przepełnić bufora, zatem zastępi ostatni znak (`'\n'`) bajtem zerowym (`'\0'`).

3. Czemu w wywołaniu printf nie dodaliśmy znaku nowej linii na końcu, a mimo to powitania wyświetlają się w oddzielnych liniach?
>Bo znak nowej linii pobierany jest z `stdin`.

4. Czemu rozmiar bufora jest MAX_LINE+2?
>Dodatkowy bajt na znak końca linii `'\n'` i jeszcze jeden na `'\0'`.

5. Jak skłonić nasz program, aby pobrał dane z pliku, a nie z klawiatury (na dwa sposoby)?
```bash
./prog3 < plik.txt
cat plik.txt | ./prog3
```

---
# Zadanie 4 — parametry wywołania programu 1
## Co student musi wiedzieć
### man 1 xargs
#### Skrót
```bash
xargs [opcje] [komenda [argumenty-początkowe]]
```

#### Opis
Polecenie `xargs` czyta ze standardowego wejścia tokeny rozdzielone spacją lub nową liniąi wywołuje `komendę` (domyślna to `/bin/echo`) raz lub więcej z `argumentami-początkowymi`, po których następują tokeny przeczytane z wejścia standardowego. Puste linie są ignorowane.

Liczba argumentów jest ograniczona przez system. `komenda` zostanie wywołana tyle razy, by zużyć wszystkie tokeny. 

Kilka argumentów:
- `-0` — tokeny kończą się `'\0'`, a nie białym znakiem oraz cudzysłów i backslash nie są traktowane specjalnie.
- `-a plik` — czytaj z pliku, a nie z `stdin`
- `-d delim` — rozdzielaj tokeny według `delim`, gdzie może to być znak, znak w stylu C (`\n`) lub oktalny czy heksadecymalny kod.
- `-L max-lines` — czyta co najwyżej `max-lines` linii z wejścia
- `-n max-args` — czyta co najwyżej `max-args` argumentów z wejścia

## Pytanka
kod `prog4.c`
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 20

int main(int argc, char **argv)
{
	int i;
	for (i = 0; i < argc; i++)
		printf("%s\n", argv[i]);
	return EXIT_SUCCESS;
}
```
kod `Makefile`
```makefile
CC=gcc
CFLAGS=-Wall -fsanitize=address,undefined
LDFLAGS=-fsanitize=address,undefined
```

Jak za pomocą programu xargs przekształcić zawartość pliku dane.txt na argumenty wywołania naszego programu?
```bash
#każdy wyraz to argument
cat plik.txt | xargs ./prog4

#każda linijka to argument
cat plik.txt | tr "\n" "\0" | xargs -0 ./prog4
```

---
# Zadanie 5 — parametry wywołania programu 2
## Co student musi wiedzieć
### man 3p exit
#### Skrót
```c
#include <stdlib.h>

void exit(int status);
```

#### Opis
Kończy proces z podanym kodem `status`. `status` może być 0, `EXIT_SUCCESS`, `EXIT_FAILURE` czy dowolną inną liczbą, ale tylko 8 LSB (`status & 0xff`) będzie dostępne dla funkcji `wait()` oraz `waitpid()`.

Funkcja `exit()` zflushuje wszystkie otwarte streamy i je zamknie. Na koniec proces zostanie zatrzymany.

### man 3p atoi
#### Skrót
```c
#include <stdlib.h>

int atoi(const char *str);
```

#### Opis
Konwertuje `str` na liczbę całkowitą.

#### Return
- Jeśli konwersja się uda, zwraca wartość.
- Jeśli nie, zachowanie jest nieokreślone (w praktyce 0).

### man 3p strtol
#### Skrót
```c
#include <stdlib.h>

long strtol(const char *restrict nptr, char **restrict endptr, int base);
```

#### Opis
Konwertuje początkowy fragment stringa wskazywanego przez `nptr` na liczbę typu `long`. String ten jest dzielony na trzy części:
1. początkowa, być może pusta, seria znaków białych `isspace()`,
2. ciąg znaków do konwersji, cyfry w systemie o podstawie `base`,
3. reszta stringa, ze znakiem `'\0'`.
Część 2. jest potem konwertowana na `longa` i zwracana.

Za `base` można podać `0`, wtedy system zostanie samodzielnie wykryty jako dziesiętny (zaczynający się cyfrą niebędącą `0`), oktalny (zaczynający się `0`) lub szesnastkowy (zaczynjący się `0x`).

Dodatkowo przez liczbą mogą znajdować się znaki `+` lub `-`.

Jeśli `endptr` nie jest `NULL`, to zostanie tam wpisany wskaźnik na pierwszy znak 3. części inputu.

Jeśli `nptr` jest pusty lub nie pasuje do konwersji, takowa nie zachodzi, `nptr` zostanie wpisany do `endptr` (jeśli nie jest `NULL`).

Jeśli wszystko przejdzie pomyślnie `errno` nie zostanie zmienione! Trzeba samemu przed konwersją dać `errno = 0`!

#### Return
- Jeśli konwersja się uda, zwraca wartość.
- Jeśli nie dało się zrobić żadnej konwersji zwraca `0` i ustawia `errno`.
- Jeśli `base` nie jest obsługiwana zwraca `0` i ustawia `errno`.
- Jeśli liczba nie mieści się w `long`, zwraca `LONG_MIN`/`LONG_MAX` i ustawia `errno`.
## Pytanka
`prog5.c`:
```c
#include <stdio.h>
#include <stdlib.h>

void usage(char *pname)
{
	fprintf(stderr, "USAGE:%s name times>0\n", pname);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	if (argc != 3)
		usage(argv[0]);
	int i, j = atoi(argv[2]);
	if (0 == j)
		usage(argv[0]);
	for (i = 0; i < j; i++)
		printf("Hello %s\n", argv[1]);
	return EXIT_SUCCESS;
}
```

1. Jak działa program dla wartości powtórzeń niepoprawnie podanych, czemu tak?
>wypisze `usage`, bo `atoi` na Linuksie zwraca 0 w przypadku niepoprawnej konwersji

2. Czemu argc ma być 3, mamy przecież 2 argumenty?
>bo pierwszy "argument" to nazwa programu

---
# Zadanie 6 — parametry wywołania programu 3
## Co student musi wiedzieć
### man 3p getopt
#### Skrót
```c
#include <unistd.h>

int getopt(int argc, char * const argv[], const char *optstring);
extern char *optarg;
extern int opterr, optind, optopt;
```
#### Opis
Funkcja `getopt()` parsuje podane do niej argumenty jako parametry POSIX-owe (typu `-n34 -t stop`). Argumenty `argc` i `argv` to odpowiednio liczba i tablica argumentów, takie jak przekazane do `main()`.  Argument `optstring` opisuje dozwolone parametry. 

Zmienna `optind` to indeks kolejnego elementu tablicy `argv[]` do przetworzenia. Domyślnie jest inicjalizowane na `1`, a `getopt()` zwiększa tę liczbę po sparsowaniu jednego parametru. `optind = 0` niezdefiniowane.

Funkcja `getopt()` zwraca znak oznaczający opcję z `optstring`. Jeśli przyjmuje ona argument, ustawiona zostanie `getopt` na wskaźnik do tego argumentu.

#### Return
- Funkcja `getopt()` zwraca następny znak oznaczający parametr z linii komend.
- Jeśli `getopt()` wykryje brakujący argument i pierwszy znak `opstring` to `':'`, zwraca `':'`
- Jeżeli `getopt()` napotka parametr, który nie został zdefiniowany lub wykryje brakujący argument i pierwszym znakiem `opstring` to 
 nie `':'`, zwraca `'?'`
 - w przeciwnym wypadku zwraca `-1`, kiedy cały command line zostanie sparsowany

#### Błędy
Jeżeli `opterr != 0`, pierwszym znakiem `optstring` nie jest `':'` oraz nastąpi błąd wpisu podczas, gdy `getopt()` wypisuje wiadomość do `stderr`, to ustawiony zostanie błąd `stream`a `stderr`, ale `getopt()` wciąż się skończy, a `errno` nie zostanie zmienione.

#### Poza POSIX
Można dać `::` co oznacza argument opcjonalny.

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void usage(char *pname)
{
	fprintf(stderr, "USAGE:%s ([-t x] -n Name ... )\n", pname);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	int c, i;
	int x = 1;
	while ((c = getopt(argc, argv, "t:n:")) != -1)
		switch (c) {
		case 't':
			x = atoi(optarg);
			break;
		case 'n':
			for (i = 0; i < x; i++)
				printf("Hello %s\n", optarg);
			break;
		case '?':
		default:
			usage(argv[0]);
		}
	if (argc > optind)
		usage(argv[0]);
	return EXIT_SUCCESS;
}
```

---
# Zadanie 7 — zmienne środowiskowe 1
## Co studenciak musi umieć?
### man 3p environ
#### Skrót
```c
extern char **environ;
```

Tablica wskaźników do zmiennych środowiskowych.

### man 7 environ
Zmienne środowiskowe w formacie `nazwa=wartość`

## Pytania kontrolne
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char **environ;

int main(int argc, char **argv)
{
	int index = 0;
	while (environ[index])
		printf("%s\n", environ[index++]);
	return EXIT_SUCCESS;
}
```

Własną zmienną mogę dodać np. tak: `TVAR2="122345" ./prog7`, pojawi się na wypisie, ale nie zostanie zapamiętana w powłoce, tzn. kolejne wywołania programu `./prog7` już jej nie pokażą.

Mogę też dodać zmienną trwale do środowiska powłoki `export TVAR1='qwert'` i teraz, ilekroć wywołam program `./prog7` ta zmienna wciąż tam będzie.

1. Czy jeśli uruchomię drugą powłokę z menu środowiska i w niej uruchomię program, to zmienna TVAR1 nadal będzie widoczna?
> Nie

2. Czy jeśli uruchomię drugą powłokę z pierwszej i w niej uruchomię program, to zmienna TVAR1 nadal będzie widoczna?
> Nie

---
# Zadanie 8 — zmienne środowiskowe 2
## Co student musi wiedzieć?
### man 3p getenv
#### Skrót
```c
#include <stdlib.h>

char *getenv(const char *name);
```

#### Opis
Funkcja ta szuka zmiennej środowiskowej o nazwie `name` i jeśli ona istnieje, to zwraca do niej wskaźnik. W przeciwnym wypadku zwraca null pointer.

#### Return
Jeśli zmienna `name` istnieje, to zwraca do niej wskaźnik. W przeciwnym wypadku zwraca null pointer.

### man 3p putenv
#### Skrót
```c
#include <stdlib.h>

int putenv(char *string);
```

#### Opis
Ustawia zmienną środowiskową podaną w formacie `name=value` w zmiennej `string`. 

#### Return
Jeśli zakończy się pomyślnie, to zwraca 0. W przeciwnym razie zwraca wartość niezerową i ustawia `errno`.

### man 3p setenv
#### Skrót
```c
#include <stdlib.h>

int setenv(const char *envname, const char *envval, int overwrite);
```

#### Opis
Aktualizuje lub dodaje zmienną środowiskową wywołującego procesu. Zmienna `envname` wskazuje na napis zawierający nazwę zmiennej. Zmienna zostanie ustawiona na wartość `envval`. Funkcja nie powiedzie się, gdy `envname` wskazuje na napis zawierający `=`. Jeśli zmienna środowiskowa `envname` już istnieje i wartość `overwrite` nie jest zerem, funkcja zwróci sukces i zmienna zostanie ustawiona. Jeśli zmienna środowiskowa `envname` już istnieje i wartość `overwrite` jest zerem, funkcja zwróci sukces i zmienna nie zostanie ustawiona. 

#### Return
W przypadku zakończenia sukcesem zwraca 0, W przeciwnym razie -1 i ustawiony zostanie `errno`.

### man 3 system
#### Skrót
```c
#include <stdlib.h>

int system(const char *command);
```

#### Opis
Używa funkcji `fork`, proces dziecko uruchamia komendę używając `execl`:
```c
execl("/bin/sh", "sh", "-c", command, (char *) NULL);
```

`system` zwraca po zakończeniu komendy. W czasie trwania wykonania zablokowany jest sygnał `SIGCHLD`, a `SIGINT` i `SIGQUIT` będzie zignorowany w procesie który wywołuje `system`. Jeśli `command` to `NULL`, to `system` zwraca informację, czy shell jest dostępny na systemie.

#### Return
Wartość zwracana przez `system` to jedno z poniższych:
- jeśli `command` to `NULL`, to niezerowa wartość oznacza, że shell jest dostępny, a 0, że nie jest dostępny.
- jeśli proces-dziecko nie mógł zostać utworzony lub jego status nie mógł być pobrany, to zwraca -1 i ustawione jest `errno`.
- jeśli shell nie mógł być uruchomiony w procesie-dziecku, to wartość zwracana odpowiada zakończeniu dziecka użyciem `_exit` z kodem 127
- jeśli wszystko się powiodło, to zwracany jest kod zakończenia procesu dziecka.

W ostatnich dwóch przypadkach zwracany jest "wait status", który może być odczytany korzystając z makr `waitpid`.

## Pytanka kontrolne
```c
#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE 20

int main(int argc, char **argv)
{
	int x, i;
	char *env = getenv("TIMES");
	if (env)
		x = atoi(env);
	else
		x = 1;
	char name[MAX_LINE + 2];
	while (fgets(name, MAX_LINE + 2, stdin) != NULL)
		for (i = 0; i < x; i++)
			printf("Hello %s", name);
	if (putenv("RESULT=Done") != 0) {
		fprintf(stderr, "putenv failed");
		return EXIT_FAILURE;
	}
	printf("%s\n", getenv("RESULT"));
	if (system("env|grep RESULT") != 0)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
```

1. Jak po wykonaniu programu sprawdzić, czy zmienna RESULT jest ustawiona? Będzie ?
```shell
env | grep RESULT
```
Nie będzie, bo ustawiana jest tylko dla procesu, nie jest propagowana wzwyż drzewa procesów.

---
# Zadanie 9 — katalogi 1
## Co student musi wiedzieć
### man 3p fdopendir
#### Skrót
```c
#include <dirent.h>

DIR *opendir(const char *dirname);
```

#### Opis
Otwiera katalog o nazwie `dirname`. Strumień katalogu ustawiony jest na pierwszy wpis. 

#### Return
W przypadku powodzenia zwraca wskaźnik na obiekt typu `DIR`. W przeciwnym razie zwraca null pointer i ustawia `errno`.

### man 3p closedir
#### Skrót
```c
#include <dirent.h>

int closedir(DIR *dirp);
```

#### Opis
Funkcja `closedir` zamyka katalog wskazany przez `dirp`.

#### Return
W przypadku powodzenia zwraca 0, w innym przypadku -1 i ustawia `errno`.

### man 3p readdir
#### Skrót
```c
#include <dirent.h>

stuct dirent *readdir(DIR *dirp);
int readdir_r(DIR *restrict dirp, struct dirent *restrict entry, struct dirent **restrict result);
```

#### Opis
Funkcja `readdir` zwraca wskaźnik na strukturę reprezentującą plik, na który wskazuje obecnie stream `dirp` i przesuwa strumień na następne miejsce. Kiedy dotrze na koniec streama `dirp`, zwraca null pointer. Struktura `dirent` opisana jest w `<dirent.h>`.

Funkcja pomija pliki z pustymi nazwami. Jeśli istnieją pliki `.` i `..` to zostaną zwrócone.

Przed użyciem należy ustawić `errno` na 0. Jeśli `errno` nie jest zerem, wystąpił błąd.

Funkcja `readdir_r` przygotuje `entry` do przechowywania informacji o aktualnym pliku wskazywanym przez `dirp`, zapisać wskażnik do tej struktury w lokalizacji wskazanej przez `result` i przesunąć `dirp` na kolejny plik.

Jeśli wszystko zakończyło się pomyślnie , `*result` będzie miał to samo co `entry`. Po dotarciu do końca katalogu będzie posiadał `NULL`.

`readdir_r` również pomija puste pliki.

#### Return
W przypadku sukcesu, `readdir` zwraca wskaźnik na obiekt typu `struct dirent`. Jeśli wystąpił błąd, zwrócony zostanie null pointer i ustawiona zmienna `errno`. Kiedy osiągnięty zostanie koniec katalogu, zwrócony zostanie null pointer i nie ustawione zostanie `errno`.

W przypadku sukcesu, `readdir_r` zwraca zero, w przeciwnym razie kod błędu.

## man 0p dirent.h
Zawartość structa `dirent`:
```c
struct dirent {
	ino_t d_ino;   // file i-number
	char d_name[]; // filename
}

int alphasort(const struct dirent **, const struct dirent **);
int closedir(DIR *);
int dirfd(DIR *);
DIR *fdopendir(int);
DIR *opendir(const char *);
struct dirent *readdir(DIR *);
int readdir_r(DIR *restrict, struct dirent *restrict,
		struct dirent **restrict);
void rewinddir(DIR *);
int scandir(const char *, struct dirent ***,
	    int (*)(const struct dirent *),
		int (*)(const struct dirent **,
		const struct dirent **));
void seekdir(DIR *, long);
long telldir(DIR *);
```

### man 3p fstatat
#### Skrót
```c
#include <fcntl.h>
#include <sys/stat.h>

int lstat(const char *restrict path, struct stat *restrict buf);
int stat(const char *restrict path, struct stat *restrict buf);
```

#### Opis
Funkcja `stat()` zbiera informacje o pliku na ścieżce `path` i zapisuje je w `buf`. **Jeśli `path` wskazuje na symlinka, to funkcja pobierze informacje o pliku wskazywanym przez tego linka!** Aktualizuje access time.

Funkcja `lstat()` nie podąża za symlinkiem, tylko zwraca informacje o nim.

Struktura `stat` opisana jest w pliku `sys/stat.h`:
```c
struct stat {
	dev_t   st_dev;   // ID of containging device
	ino_t   st_ino;   // File i-node
	mode_t  st_mode;  // File mode (regular, directory, symlink, etc)
	nlink_t st_nlink; // no of hard links
	uid_t   st_uid;   // UID of owner
	gid_t   st_gid;   // GID of owner
	dev_t   st_rdev;  // Device ID (if character or block)
	off_t   st_size;  // For regular filer: file size in bytes
					  // For symlinks: length of pathname
					  // For shared/typed memory object: length in bytes
					  // Otherwise: unspecified

	struct timespec st_atim; // Last data access
	struct timespec st_mtim; // Last data modification
	struct timespec st_ctim; // Last file status change

	blksize_t st_blksize; // I/O block size
	blkcnt_t  st_blocks;  // No of blocks
}
```

#### Return
W przypadku powodzenia zwraca 0. W przypadku błędu -1 i ustawia `errno`.

### man 3p errno
Zmienna `errno` zawiera ostatni kod błędu.  

### man 2 lstat
#### Makra
- `S_ISREG` — regular file
- `S_ISDIR` — directory
- `S_ISCHR` — character device
- `S_ISBLK` — block device
- `S_ISFIFO` — FIFO
- `S_ISLINK` — symlink
- `S_ISSOCK` — socket

## Pytania kontrolne
```c
void scan_dir() {
    DIR * dirp;
    struct dirent * dp;
    struct stat filestat;
    int dirs = 0, files = 0, links = 0, other = 0;
    if (NULL == (dirp = opendir(".")))
        ERR("opendir");
    do {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL) {
            if (lstat(dp -> d_name, & filestat))
                ERR("lstat");
            if (S_ISDIR(filestat.st_mode))
                dirs++;
            else if (S_ISREG(filestat.st_mode))
                files++;
            else if (S_ISLNK(filestat.st_mode))
                links++;
            else
                other++;
        }
    } while (dp != NULL);

    if (errno != 0)
        ERR("readdir");
    if (closedir(dirp))
        ERR("closedir");
    printf("Files: %d, Dirs: %d, Links: %d, Other: %d\n", files, dirs, links, other);

}
```

1. Uruchom ten program w katalogu, w którym masz jakieś pliki, może być ten, w którym wykonujesz ten tutorial, ważne aby nie było w nim katalogów, czy wyniki zgadzają się z tym czego oczekujemy tj. zero katalogów, trochę plików?
> Nie, bo są katalogi `.` i `..`

2. Jak utworzyć link symboliczny do testów ?
```sh
ln -s prog9.c prog_link.c
```

3. Jak różnią się `stat` i `lstat`? Czy jeśli w kodzie zmienić `lstat` na stat to zliczymy linki poprawnie ?
> `stat` pobierze informacje o pliku, na który wskazuje symlink, więc dostaniemy 0 symlinków

4. Jakie pola zawiera struktura opisująca obiekt w systemie plików (`dirent`) wg. POSIX ?
> Numer i-node'a i nazwę.

5. Jakie pola zawiera struktura opisująca obiekt w systemie plików (dirent) w Linuksie (man readdir) ?
> Numer i-node'a, nazwę i trzy inne poza POSIX

---
# Zadanie 10 — katalogi 2
## Co student musi wiedzieć:
### man 3p getcwd
#### Skrót
```c
#include <unistd.h>

char *getcwd(char *buf, size_t size);
```

#### Opis
Funkcja ta umieszcza absolutną ścieżkę `cwd` w buforze `buf` i zwraca `buf`.

#### Return
W przypadku powodzenia zwraca `buf`, w przeciwnym razie null pointer i ustawia `errno`.

### man 3p chdir
#### Skrót
```c
#include <unistd.h>

int chdir(const char *path);
```

#### Opis
Zmienia cwd na `path`.

#### Return
W przypadku powodzenia zwraca 0, w przeciwnym razie -1 i ustawia `errno`.

## Pytania kontrolne
```c
int main(int argc, char **argv)
{
	int i;
	char path[MAX_PATH];
	if (getcwd(path, MAX_PATH) == NULL)
		ERR("getcwd");
	for (i = 1; i < argc; i++) {
		if (chdir(argv[i]))
			ERR("chdir");
		printf("%s:\n", argv[i]);
		scan_dir();
		if (chdir(path))
			ERR("chdir");
	}
	return EXIT_SUCCESS;
}
```
1. Czemu program pobiera i zapamiętuje aktualny katalog roboczy?
> Bo ścieżki mogły być względne

2. Czy prawdziwe jest stwierdzenie, że program powinien „wrócić” do tego katalogu, w którym był uruchomiony?
> Nie, nie ma takiej potrzeby

3. W tym programie nie wszystkie błędy muszą zakończyć się wyjściem, który można inaczej obsłużyć i jak?
> Jeśli `chdir(argv[i])` się nie powiedzie, nie trzeba wyrzucać błędu.

---
# Zadanie 11 — katalogi 3
## Co student musi wiedzieć
### man 3p ftw
#### Skrót
```c
#include <ftw.h>

int ftw (const char *path, int (*fn)(const char *, const struct stat *ptr, int flag), int ndirs);
```

#### Opis
Funkcja `ftw` rekurencyjnie przejdzie po hierarchii folderów zaczynając w `path`. Dla każdego obiektu w hierarchii, `ftw` wywoła funkcję `fn` przekazując jej nazwę obiektu, wzkaźnik na strukturę `stat` zawierającą informacje o obiekcie (tak jak `stat` lub `lstat`). Jako `flag` mogą być przekazane:
- `FTW_D` — katalog
- `FTW_DNR` — katalog, który nie może być odczytany
- `FTW_F` — niekatalog
- `FTW_SL` — symlink
- `FTW_NS` — obiekt niebędący symlinkiem, na którym nie dało się wykonać `stat`.
Argument `ndirs` określa maksymalną liczbę otwartych katalogów. 

#### Return
Jeśli skończy się drzewo katalogów, zwrócone zostanie 0. Jeśli funkcja `fn` zwróci niezero, `ftw` zakończy przeglądanie plików i zwróci to co `fn`. Jeśli `ftw` wykryje błąd, zwróci `-1` i ustawi `errno`.

### man 3p ntfw
Na linuksie wymaga `#define _XOPEN_SOURCE 500`
#### Skrót
```c
#include <ftw.h>

int nftw(const char *path, int (*fn)(const char *, const struct stat *, int, struct FTW *), int fd_limit, int flags);
```

#### Opis
Funkcja `nftw` rekurencyjnie przejdzie po hierarchii folderów zaczynając w `path`. Funkcja `nftw` działa podobnie jak `ftw`, jednak przyjmuje dodatkowy argument `flags`, będący ORem następujących flag:
- `FTW_CHDIR` — `nftw` zmieni `cwd` na każdy z katalogów, podczas gdy mówi o plikach z tego katalogu. W przeciwnym razie zostanie w `cwd`.
- `FTW_DEPTH` — `nftw` najpierw powie o plikach w katalogu, a dopiero potem o samym katalogu. W przeciwnym razie najpierw powie o katalogu, a potem o plikach w nim
- `FTW_MOUNT` — `nftw` powie tylko o plikach w tym samym systemie plików co `path`. W przeciwnym razie powie o wszystkich plikach
- `FTW_PHYS` — `nftw` przejdzie "fizycznie" po plikach, nieprzechodząc przez symlinki.

Argumenty funkcji `fn`:
- pierwszy argument to ścieżka do obiektu
- drugi argument to wskaźnik na `stat` wypełniony tak jak `stat` lub `lstat`
- trzeci argument to flaga:
	- `FTW_D` — katalog
	- `FTW_DNR` — katalog, który nie może być odczytany
	- `FTW_DP` — katalog i jego podkatalogi zostały odiwedzone
	- `FTW_F` — niekatalog
	- `FTW_NS` — `stat` nie powiódł się
	- `FTW_SL` — symlink
	- `FTW_SLN` — symlink do nieistniejącego pliku
- czwarty argument to wskaźnik na strukturę `FTW`: `base` mówi o offsecie nazwy obiektu w ścieżce, `level` mówi o zagłębieniu w drzewo katalogów

`fd_limit` mówi o maksymalnej liczbie otwartych deskryptorów.

#### Return
Funkcja trwa, dopóki nie zajdzie któryś z trzech warunków
- `fn` zwróciło coś innego niż zero, `nftw` zwaraca tę wartość
- `nftw` wykryło błąd, zwraca -1 i ustawia `errno`
- drzewo katalogów się skończyło, zwaraca 0

## Pytania kontrolne
```c
#define _XOPEN_SOURCE 500
#include <dirent.h>
#include <errno.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#define MAXFD 20

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

int dirs = 0, files = 0, links = 0, other = 0;

int walk(const char *name, const struct stat *s, int type, struct FTW *f)
{
	switch (type) {
	case FTW_DNR:
	case FTW_D:
		dirs++;
		break;
	case FTW_F:
		files++;
		break;
	case FTW_SL:
		links++;
		break;
	default:
		other++;
	}
	return 0;
}

int main(int argc, char **argv)
{
	int i;
	for (i = 1; i < argc; i++) {
		if (nftw(argv[i], walk, MAXFD, FTW_PHYS) == 0)
			printf("%s:\nfiles:%d\ndirs:%d\nlinks:%d\nother:%d\n", argv[i], files, dirs, links, other);
		else
			printf("%s: brak dostępu\n", argv[i]);
		dirs = files = links = other = 0;
	}
	return EXIT_SUCCESS;
}
```
1. W jakim celu użyta jest flaga `FTW_PHYS`?
> Żeby nie wchodzić w symlinki, tylko powiedzieć o nich.

---
# Zadanie 12 — operacje na plikach
## Co student musi wiedzieć?
### man 3p fopen
#### Skrót
```c
#include <stdio.h>

FILE *fopen(const char *restrict pathname, const char *restrict mode);
```

#### Opis
Otwiera plik o ścieżce `pathname` w trybie `mode`, gdzie:
- `r`/`rb` — odczyt
- `w`/`wb` — wyczyszczenie lub utworzenie pliku i zapis
- `a`/`ab` — dopisanie do pliku
- `r+`/`rb+`/`r+b` — odczyt i zapis
- `w+`/`wb+`/`w+b` — wyczysczenie lub utworzenie pliku, odczyt i zapis
- `a+`/`ab+`/`a+b` — dopisanie do pliku, odczyt

#### Return
W przypadku sukcesu zwraca wskaźnik na `FILE`.  W przypadku błędu zwraca null pointer i ustawia `errno`.

### man 3p fclose
#### Skrót
```c
#include <stdio.h>

int fclose(FILE *stream);
```

#### Opis
Flushuje i zamyka plik `stream`. Aktualizuje czasy modifikacji i zmiany pliku.

#### Return
W przypadku sukcesu zwraca 0, w przeciwnym razie `EOF` i ustawia `errno`.

### man 3p fseek
#### Skrót
```c
#include <stdio.h>

int fseek(FILE *stream, long offset, int whence);
```

#### Opis
Funkcja `fseek()` ustawia pozycję w pliku `stream`. Nowa pozycja (w bajtach) otrzymana zostanie przez dodanie `offset` do `whence`, gdzie:
- `SEEK_SET` — początek pliku
- `SEEK_CUR` — obecna pozycja
- `SEEK_END` — koniec pliku

#### Return
W przypadku sukcesu zwraca 0, w przeciwnym wypadku -1 i ustawia `errno`.

### man 3p rand
#### Skrót
```c
#include <stdlib.h>

inr rand(void);
void srand(unsigned seed);
```

#### Opis
Funkcja `rand` oblicza liczbę pseudolosową w zakresie $[0, \{\verb|RAND_MAX|\}]$. Funkcja `srand` ustawia `seed` generatora.

#### Return
`rand` zwraca pseudolosową liczbę, `srand` — nic.

### man 3p unlink
#### Skrót
```c
#include <unistd.h>

int unlink(const char *path);
```

#### Opis
Funkcja `unlink` usuwa link do pliku. Jeśli `path` to symlink, to usuwa go i nijak nie zmienia pliku wskazywanego przez symlink. W przeciwnym razie, `unlink` usunie linka na `path` i zmniejszy liczbę linków pliku wskazywanego przez linka.

Kiedy liczba linków pliku osiągnie 0 i żaden proces nie będzie z niego korzystał, miejsce po nim zostanie zwolnione i przestanie być dostępny. `unlink` zmodyfikuje odpowiednie znaczkini czasowe.

#### Return
W przypadku sukcesu zwraca 0, w przeciwnym razie `-1` i ustawi `errno`.

### man 3p umask
#### Skrót
```c
#inlcude <sys/stat.h>

mode_t umask(mode_t cmask);
```

#### Opis
Funkcja `umask` ustawia umaskę procesu na `cmask` i zwraca wartość poprzedniej umaski. Tylko bity uprawień plików są używane. Umaska brana jest pod uwagę przy wywołaniu `open`, `openat`, `creat`, `mkdir`, `mkdirat`, `mkfifo`, `mkfifoat`, `mknod`, `mknodat` itp

#### Return
Poprzednia umaska.

## Pytania kontrolne
```c
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage(char *pname)
{
	fprintf(stderr, "USAGE:%s -n Name -p OCTAL -s SIZE\n", pname);
	exit(EXIT_FAILURE);
}

void make_file(char *name, ssize_t size, mode_t perms, int percent)
{
	FILE *s1;
	int i;
	umask(~perms & 0777);
	if ((s1 = fopen(name, "w+")) == NULL)
		ERR("fopen");
	for (i = 0; i < (size * percent) / 100; i++) {
		if (fseek(s1, rand() % size, SEEK_SET))
			ERR("fseek");
		fprintf(s1, "%c", 'A' + (i % ('Z' - 'A' + 1)));
	}
	if (fclose(s1))
		ERR("fclose");
}

int main(int argc, char **argv)
{
	int c;
	char *name = NULL;
	mode_t perms = -1;
	ssize_t size = -1;
	while ((c = getopt(argc, argv, "p:n:s:")) != -1)
		switch (c) {
		case 'p':
			perms = strtol(optarg, (char **)NULL, 8);
			break;
		case 's':
			size = strtol(optarg, (char **)NULL, 10);
			break;
		case 'n':
			name = optarg;
			break;
		case '?':
		default:
			usage(argv[0]);
		}
	if ((NULL == name) || ((mode_t)-1 == perms) || (-1 == size))
		usage(argv[0]);
	if (unlink(name) && errno != ENOENT)
		ERR("unlink");
	srand(time(NULL));
	make_file(name, size, perms, 10);
	return EXIT_SUCCESS;
}
```

1. Jaką maskę bitową tworzy wyrażenie `~perms&0777`?
> Odwrotność wymaganych uprawnień podanych w `perms`, bo tak działa umaska.

2. Jak działa losowanie znaków?
> W losowych miejscach pliku wstawia kolejne litery alfabetu.

3. Uruchom program kilka razy, pliki wynikowe wyświetl poleceniem cat i less sprawdź, jakie mają rozmiary (ls -l), czy zawsze równe podanej w parametrach wartości? Z czego wynikają różnice dla małych rozmiarów -s a z czego dla dużych (> 64K) rozmiarów?
> Nie zawsze ostatni znak trafi na ostatnią pozycję + `RAND_MAX` jest dwubajtowy.

4. Czemu podczas sprawdzania błędu unlink jeden przypadek ignorujemy?
> ENOENT oznacza brak pliku, jeśli plik o podanej nazwie nie istniał, to nie możemy go skasować, ale to nie przeszkadza programowi, w tym kontekście to nie jest błąd. Bez tego wyjątku moglibyśmy tylko nadpisywać istniejące pliki a nie tworzyć nowe.

---
# Zadanie 13 — buforowanie standardowego wyjścia
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    for (int i = 0; i < 15; ++i) {
        // Output the iteration number and then sleep 1 second.
        printf("%d\n", i);
        sleep(1);
    }
    return EXIT_SUCCESS;
}
```
1. Spróbuj uruchomić ten (bardzo prosty!) kod z terminala. Co widać na terminalu?
> Co sekundę liczba

2. Spróbuj uruchomić kod ponownie, tym razem jednak przekierowując wyjście do pliku `./plik_wykonwyalny > plik_z_wyjściem`. Następnie spróbuj otworzyć plik z wyjściem w trakcie działania programu, a potem zakończyć działanie programu przez Ctrl+C i otworzyć plik jeszcze raz. Co widać tym razem?
> Jeśli zrobimy te kroki wystarczająco szybko, plik okazuje się pusty! To zjawisko wynika z tego, że biblioteka standardowa wykrywa, że dane nie trafiają bezpośrednio do terminala, i dla wydajności buforuje je, zapisując je do pliku dopiero, gdy zbierze się ich wystarczająco dużo. To oznacza, że dane nie są dostępne od razu, a w razie nietypowego zakończenia programu (tak jak kiedy użyliśmy Ctrl+C) mogą wręcz zostać stracone. Oczywiście, jeśli damy programowi dojść do końca działania, to wszystkie dane zostaną zapisane do pliku (proszę spróbować!). Mechanizm buforowania można skonfigurować, ale nie musimy tego robić, jak za chwilę zobaczymy.

3. Spróbuj uruchomić kod podobnie, ponownie pozwalając wyjściu trafić do terminala (jak za pierwszym razem), ale spróbuj usunąć nową linię z argumentu `printf`: `printf("%d", i);`. Co widzimy tym razem?
> Wbrew temu, co powiedzieliśmy wcześniej, nie widać wyjścia mimo to, że tym razem dane trafiają bezpośrednio do terminala; dzieje się natomiast to samo co w poprzednim kroku. Otóż biblioteka buforuje standardowe wyjście, nawet jeśli dane trafiają do terminala; jedyną różnicą jest to, że reaguje na znak nowej linii, wypisując wszystkie dane zebrane w buforze. To ten mechanizm sprawił, że w pierwszym kroku nie wydarzyło się nic dziwnego. Właśnie dlatego czasami zdarza się Państwu, że `printf` nie wypisuje nic na ekran; jeśli zapomnimy o znaku nowej linii, standardowa biblioteka nic nie wypisze na ekran dopóki w innym wypisywanym stringu nie pojawi się taki znak, lub program się nie zakończy poprawnie.

4. Spróbuj ponownie zrobić poprzednie trzy kroki, tym razem jednak wypisując dane do strumienia standardowego błędu: `fprintf(stderr, /* parametry wcześniej przekazywane do printf */);`. Co dzieje się tym razem? Żeby przekierować standardowy błąd do pliku, należy użyć `>2` zamiast `>`.
> Tym razem nic się nie buforuje i zgodnie z oczekiwaniami widzimy jedną cyfrę co sekundę. Standardowa biblioteka nie buforuje standardowego błędu, często bowiem wykorzystuje się go do debugowania.
