/*
Yapılacaklar:
*Uygulamayı aç
*Dosya ismi al
*Dosyayı okumak için aı
*Dosyada aranacak kelime, harf al.
*Her satırı tek tek oku
Eıer bulunduysa:
*Bulunduıu satır, satır baıına uzaklık olarak  bir log dosyasına yaz.
Bulunamadıysa:
*Programı bitir.
*Dosyanın sonuna aranan kelimeden az kaldıysa bitir.
*Dosya sonu gelince, dosyayı kapat.
*Kullandııın deıiıkenleri temizle.
*Programı kapat.

struct:
	found counter
	found position pair
*/
#include "List.h"
void makeCharLover(char * inputChar)
{
	if (*inputChar >= 'A' && *inputChar <= 'Z')
		*inputChar += 'a' - 'A';
	return;
}

const char * const makeStrLover(char * const inputStr)
{
	int charCounter = 0;
	while (*(inputStr + charCounter) != 0)
		makeCharLover(inputStr + charCounter++);
	return inputStr;
}

int searchOn(int openedFile, const char * const stringtaken)
{
	int counter = 0, sameParts = 1, error = 0;
	char char_in;
	while (sameParts != strlen(stringtaken) && error == 0)
	{
		read(openedFile, &char_in, sizeof(char));
		++counter;
		if (char_in == '\n' ||
			char_in == '\t' ||
			char_in == ' ');
		else
		{
			makeCharLover(&char_in);			
			if (char_in == *(stringtaken + sameParts))
				++sameParts;
			else 
				error = 1;
		}
	}
	lseek(openedFile, -(counter * sizeof(char)), SEEK_CUR);
	return (error + 1) % 2;
}

int searchfile(int openedFile, const char * const stringTaken)
{
	char single_char;
	int row = 1, /*Satır*/
		col = 1,/*Sıtun*/
		foundTimes = 0;
	while (read(openedFile, &single_char, sizeof(char)))
	{
		/*
			Eıer yeni satır karakteri gelirse,
			rowinLine deıiıkeni arttırılıp,
			col 1 ile deıiıtiriliyor.
		*/
		if (single_char == '\n')
		{
			++row;
			col = 0;
		}
		else if(single_char == '\t' ||
				single_char == ' ');
		/*
			Eıer okunan karakter,
			aranan karakter ile aynı ise;
			Aranan kelimenin bıtını var mı diye
			kontrol etmeye gidiyor.
		*/		
		else 
		{
			makeCharLover(&single_char);
			if (single_char == *stringTaken)
				if (searchOn(openedFile, stringTaken) == 1 && ++foundTimes)
					printf("[%d, %d] konumunda ilk harf bulundu.\n", row, col);
		}
		++col;
	}
	printf("\n\n%d adet %s bulundu.\n", foundTimes, stringTaken);
	return 0;
}