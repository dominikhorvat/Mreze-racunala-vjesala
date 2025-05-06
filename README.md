# Mreze-racunala-vjesala

Napišite mrežne aplikacije (server i klijent) koje će omogućavati igranje igre "vješala". Server sadrži riječ koju je
 potrebno pogoditi. Klijenti se spajaju na server i natječu tko će pogoditi riječ u manje pokušaja.
 Koraci u igri:
 Klijenti se spajaju na server u bilo kojem redosljedu. Odmah nakon što se neki klijent spoji, server mu
 šalje riječ u kojoj su sva slova sakrivena, te preostali broj pokušaja. Skrivena slova prikazujemo znakom
 '?'.
 Nakon primanja, klijentski program prikazuje riječ i broj pokušaja. Klijent zatim serveru šalje neko slovo.
 Ukoliko je klijent pogodio slovo koje se nalazi u riječi, to slovo je otkriveno tokom sljedećeg slanja. Npr.
 za riječ "otorinolaringologija", server prvo klijentu šalje niz znakova "????????????????????" (20 znakova
 '?'). Ako klijent pošalje slovo 'o', sljedeći niz znakova koje server šalje klijentu je "o?o???o??????o?o????".
 Ukoliko klijent nije pogodio slovo, smanjuje mu se broj preostalih pokušaja.
 Slanje riječi i broja pokušaja se ponavlja sve dok klijent ne potroši sve svoje pokušaje ili pogodi riječ.
 Nakon što igra završi, server klijentu šalje njegovu poziciju u rang listi koja sadrži podatke o svim
 završenim igrama.
 Rang klijenta određuje se na sljedeći način: klijenti koji nisu pogodili riječ su uvijek niže rangirani od onih
 koji jesu. Ukoliko više klijenata pogodi riječ, najbolji je onaj kojem je preostalo najviše pokušaja. Klijenti
 koji nisu pogodili rangirani su prema broju slova koja su otkrili. Sami odredite kako ćete rješiti slučaj kad
 klijenti dijele poziciju.
 Napomena:
 Napomena: Klijenti moraju moći igrati igru istovremeno i neovisno jedni o drugima (koristite pthreads). Uočite
 da za određivanje ranga ne morate razlikovati klijente (naravno, ako želite, možete).
