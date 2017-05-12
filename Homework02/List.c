/*
Yapılacaklar:
*Uygulamayı aç
*Dosya ismi al
*Dosyayı okumak için aç
*Dosyada aranacak kelime, harf al.
*Her satırı tek tek oku
Eğer bulunduysa:
*Bulunduğu satır, satır başına uzaklık olarak  bir log dosyasına yaz.
Bulunamadıysa:
*Programı bitir.
*Dosyanın sonuna aranan kelimeden az kaldıysa bitir.
*Dosya sonu gelince, dosyayı kapat.
*Kullandığın değişkenleri temizle.
*Programı kapat.
*/
#include "List.h"
char makeCharLover(char inputChar)
{
	if (inputChar >= 'A' && inputChar <= 'Z')
		inputChar += 'a' - 'A';
	else;
	return inputChar;
}

char *makeStrLover(char *inputStr)
{
	int charCounter = 0;
	while (charCounter < strlen(inputStr))
	{
		inputStr[charCounter] = makeCharLover(inputStr[charCounter]);
		++charCounter;
	}
	return inputStr;
}

int doesStringFound(FILE *openedFile, const char stringtaken[])
{
	int readCharactersFromFile = 0, sameParts = 1, error = 0;
	char charfromFile;
	fpos_t pos;
	fgetpos(openedFile, &pos);
	while (sameParts != strlen(stringtaken) && error != 1)
	{
		charfromFile = fgetc(openedFile);
		charfromFile = makeCharLover(charfromFile);
		++readCharactersFromFile;
		if (charfromFile == '\n' || charfromFile == '\t' || charfromFile == ' ');
		else if (charfromFile == stringtaken[sameParts])
			++sameParts;	
		else if (charfromFile == EOF || charfromFile != stringtaken[sameParts])
			error = 1;
	}
	fsetpos(openedFile, &pos);
	if (error == 0)
		return 1;
	return 0;
}

int searchOpenedFile(FILE *openedFile, const char stringTaken[])
{
	char oneCharFromFile;
	int rowinFile = 1, /*Satır*/
		columninFile = 1,/*Sütun*/
		foundTimes = 0;
	do{
		/*
			Dosyadan ilk harf okunuyor.
		*/
		oneCharFromFile = makeCharLover(fgetc(openedFile));
		/*
			Eğer yeni satır karakteri gelirse,
			rowinLine değişkeni arttırılıp,
			columninFile 1 ile değiştiriliyor.
		*/
		if (oneCharFromFile == '\n')
		{
			++rowinFile;
			columninFile = 0;
		}
		/*
			Eğer okunan karakter,
			aranan karakter ile aynı ise;
			Aranan kelimenin bütünü var mı diye
			kontrol etmeye gidiyor.
		*/
		else if (oneCharFromFile == stringTaken[0])
		{
			if (doesStringFound(openedFile, stringTaken) == 1)
			{
				++foundTimes;
				printf("[%d, %d] konumunda ilk harf bulundu.\n", rowinFile, columninFile);
			}
			else
				;
			}
		++columninFile;
	} while (oneCharFromFile != EOF);
	printf("\n\n%d adet %s bulundu.\n", foundTimes, stringTaken);
	return 0;
}