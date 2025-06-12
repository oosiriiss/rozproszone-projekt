
# Implementacja gry sieciowej z przedmiotu przetwarzanie rozproszone

### Wymagania:
- Komputer z systemem Linux. (Testy przeprowadzane były na urządzeniu z systemem Arch linux)
- **cmake** - do generowania plików budowy projektu
- **ninja** - Build system (opcjonalne w przypadku manualne kompilacji)
- **tmux** - Do wygodnego właczenia 3 instancji aplikacji w jednym oknie terminala


### Uruchomienie za pomocą skryptu

Aplikacja może zostać uruchomienia poprzez wykonanie poniższego polecenia, które samo skompiluje projekt oraz otworzy okna aplikacji.

Wewnątrz folderu pobranego z repozytoriumu uruchomić polecenia (skrypt ./run.sh musi posiadać uprawnienia do wykonywania):
```
mkdir build && cd build && cmake .. -G Ninja && ./run.sh
```


### Kompilacja manualna

1. Klonowanie repozytorium
```
$ git clone rozproszone-projekt && git checkout master && cd rozproszone-projekt
```
2. Generowanie plików do kompilacji (cmake + ninja)
```
$ mkdir build && cd build && cmake .. -G Ninja 
```
3. Kompilacja
```
$ ninja
```
4. Po pomyślnej kompilacji w katalogu build powinny znajdować się 2 pliki wykonywalne **executable-debug** oraz **executable-release**.

### Uruchomienie manualne

1. Po pomyślnej kompilacji należy upewnić się, że port **63921** nie jest zajęty przez inny proces (Aplikacja używa tego portu dla serwera). Port ten można zmienić w pliku **Application.cpp** w funkcji ```Application::run```, a następnie ponownie skompilować projekt.
2. Uruchomić 3 instancje aplikacji za pomocą poniższych poleceń (pliki wykonywanlne to ./build/executable-debug oraz ./build/executable-release):
    1. Okno serwerowe ```./build/executable-release server```
    2. Okno Klienta 1 ```./build/executable-release```
    3. Okno Klienta 2 ```./build/executable-release```
