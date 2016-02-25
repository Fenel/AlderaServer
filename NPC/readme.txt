Dostêpne akcje:
actRemovePlayerItem(itemid,count) - zwraca 1 na stos gdy siê uda, 0 gdy siê nie uda
actAddPlayerItem(itemid,count) - zwraca 1 na stos gdy siê uda, 0 gdy siê nie uda
actGetPlayerValue(key,0) - wartoœæ zostaje pobrana na stos, gdy brak danego klucza, zwraca na stos -1
actSetPlayerValue(key,value) - ustawia wartoœæ podanego klucza, na stos zawsze zwraca 1
actAddPlayerExperience(amount,0) - zwraca 1 na stos gdy siê uda, 0 w przeciwnym przypadku



Wyra¿enia warunkowe 'if':

Aby u¿yæ wyra¿enia warunkowego 'if', nale¿y wczeœniej wykonaæ jak¹œ akcjê, gdy¿ 'if' sprawdza wynik ostatniej akcji.
np.

actAddPlayerExperience(1000,0) #dodaj graczowi 1000 doœwiadczenia

if 1 #je¿eli poprzednia operacja zakoñczy³a siê sukcesem
then
   say "Dodalem exp"
else
   say "Coœ posz³o nie tak, nie doda³em exp"
fi
