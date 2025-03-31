# Rajzfelismerő

A program lényege, hogy egyszerűbb többrétegű perceptron típusú neurális hálózatokat képes beolvasni fájlból, majd azokat futtatni tudja a felhasználó által megadott bemenet szerint.

Bemenet megadásához a program kínál egy rajztáblát, amin lehet ecset, ceruza és radír eszközökkel rajzolni, és aminek felbontását a betöltött modell határozza meg.
Megfelelő modell betöltése után így egy "rajzfelismerő" programot kaphatunk.

Bármilyen bemenet megadása után átléphetünk egy olyan megjelenítési módba, ahol a teljes modell van síkba rajzolva, és ahol böngészni tudjuk a modell bármelyik neuronjának értékeit és a hozzájuk tartozó kapcsolatokat.

![rajzfelismero](https://github.com/user-attachments/assets/b44293af-6376-4155-a4d6-26da3e90b742)

### Használat
A program által támogatott fájlkiterjeztés a **.mplmodel**, aminek a belső formátuma részletezve van a [specifikációban](specifikacio.pdf).

Pár előkészített modell:
- [96.mlpmodel](96.mlpmodel) (kisméretű modell, 0-9 számjegyekre, 28x28-as táblaméret)
- [full_28x28_1x1_128_64.mlpmodel](full_28x28_1x1_128_64.mlpmodel) (0-9 számjegyekre, 56x56-os táblaméret)
- [qmnist.mlpmodel](qmnist.mlpmodel) (0-9 számjegyekre, QMNIST adathalmazból, 28x28-as táblaméret)
- [test.mlpmodel](test.mlpmodel) (0-9 számjegyekre teszt, pontatlan, 28x28-as táblaméret)
- [horizontal.mlpmodel](horizontal.mlpmodel) (rajztábla vízszintes igazítására teszt)
- [vertical.mlpmodel](vertical.mlpmodel) (rajztábla függőleges igazítására teszt)
- [letters.mlpmodel](letters.mlpmodel) (nem működő kezdeti próba a betűk felismerésére is)
