# Witam zapraszam do kolaboracji

Powinno dzialac tylko na linuxie bo uzywam unixowych socketow ale jak to jest na macu to idk. Jak ktos chce to moze dodac support windowsa bo api prawie takie samo jest

jest cmake skonfigurowanyto mozecie skompilowac w root katalogu tym (nie trzeba uzywac Ninja ale jak nie wiecie to zainstalujcie to i tak bedzie najlatwiej (na ```cmake --help``` macie na dole wszyskie mozliwe opcje)

```
mkdir build && cd build && cmake .. -G Ninja && ninja 
```
plik wykonywalny sie znajdowac w root_katalog/bin

tez jest jakis skrypt to mozecie se wpisywac ```./run.sh``` i powinno dzialac


### jak dodacie jakies pliki to trzeba je dodac do CMakeLists.txt tam gdzie reszta plikow cpp w target_source





