Dost�pne akcje:
actRemovePlayerItem(itemid,count) - zwraca 1 na stos gdy si� uda, 0 gdy si� nie uda
actAddPlayerItem(itemid,count) - zwraca 1 na stos gdy si� uda, 0 gdy si� nie uda
actGetPlayerValue(key,0) - warto�� zostaje pobrana na stos, gdy brak danego klucza, zwraca na stos -1
actSetPlayerValue(key,value) - ustawia warto�� podanego klucza, na stos zawsze zwraca 1
actAddPlayerExperience(amount,0) - zwraca 1 na stos gdy si� uda, 0 w przeciwnym przypadku



Wyra�enia warunkowe 'if':

Aby u�y� wyra�enia warunkowego 'if', nale�y wcze�niej wykona� jak�� akcj�, gdy� 'if' sprawdza wynik ostatniej akcji.
np.

actAddPlayerExperience(1000,0) #dodaj graczowi 1000 do�wiadczenia

if 1 #je�eli poprzednia operacja zako�czy�a si� sukcesem
then
   say "Dodalem exp"
else
   say "Co� posz�o nie tak, nie doda�em exp"
fi
