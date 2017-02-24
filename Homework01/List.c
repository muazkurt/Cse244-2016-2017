#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define StdSearch 1024
/*
Yapýlacaklar:
	*Uygulamayý aç
	*Dosya ismi al
	*Dosyayý okumak için aç
	*Dosyada aranacak kelime, harf al.
	*Her satýrý tek tek oku
	Eðer bulunduysa:
		*Bulunduðu satýr, satýr baþýna uzaklýk olarak  bir log dosyasýna yaz.
	Bulunamadýysa:
		*Programý bitir.
	*Dosyanýn sonuna aranan kelimeden az kaldýysa bitir.
	*Dosya sonu gelince, dosyayý kapat.
	*Kullandýðýn deðiþkenleri temizle.
	*Programý kapat.
*/

/*
	->Tries opening given file for Reading. If file can' t open,
	->Prints an error message and returns -1.
*/
int openGivenFile(FILE *inputFile, char fileName[]);

int searchOpenedFile(FILE *openedFile, char stringTaken[]);

int main(int argv, char * argc[])
{
	FILE *FileWillSearchen;
	/*Usage part of the tool.*/
	if (argv == 1)
	{
		fprintf(stderr, "Usage of the 'List': \n");
		fprintf(stderr, "\t->List \"String\" <Filename>\n");
		fprintf(stderr, "\t->This tool takes a String and File Name,\n");
		fprintf(stderr, "\t\tsearches the String inside of the taken File.\n");
		fprintf(stderr, "\t->Each time if found it, prints where it found\n");
		fprintf(stderr, "\t\tlike;\n\t->Times of Found: ~, Row: ~, Column: ~\n");
		return -1;
	}
	/*Error situation.*/
	else if(argv != 3)
	{
		fprintf(stderr, "Invalid input for tool.\n");
		fprintf(stderr, "Please use like this: \n");
		fprintf(stderr, "\t->List \"String\" <Filename>\n");
		return -1;
	}
	/*Working part of the tool.*/
	else
	{
		if (openGivenFile(FileWillSearchen, argc[2]) == 0)
			searchOpenedFile(FileWillSearchen, argc[1]);
		else
			return -1;
	}
	return 0;
}

int openGivenFile(FILE *inputFile, char fileName[])
{
	inputFile = fopen(fileName, "r");
	if (inputFile == NULL)
	{
		fprintf(stderr, "Error while openning file.\n");
		fclose(inputFile);
		return -1;
	}
	else if (feof(inputFile))
	{
		printf("The file is empty.\n");
		fclose(inputFile);
		return -1;
	}
	return 0;
}

int searchOpenedFile(FILE *openedFile, char stringTaken[])
{
	char oneCharFromFile, spaceSymbol = ' ';
	int rowinFile = 1, /*Satýr*/
		columninFile = 1,/*Sütun*/
		everyFoundPart = 0, 
		unexpectedTokens = 0, 
		foundTimes = 0;
	do{
		
		/*
			Dosyadan ilk harf okunuyor.
		*/
		fscanf(openedFile, "%c", &oneCharFromFile);
		
		/*
			Eðer yeni satýr karakteri gelirse,
			rowinLine deðiþkeni arttýrýlýp,
			columninFile 1 ile deðiþtiriliyor.
		*/
		if (oneCharFromFile == '\n')
		{
			++rowinFile;
			columninFile = 1;
		}
		
		/*
			Eðer okunan karakter,
			aranan karakter ile ayný ise;
			Aranan kelimenin bütünü var mý diye
			kontrol etmeye gidiyor.
		*/
		else if (oneCharFromFile == stringTaken[everyFoundPart])
		{

			/*
				Aranan ilk harf bulunduðuna göre,
				Aranan kelime, dosya içinde harf harf bakýlýyor.
				Whitespace karakterleri yok sayýlýyor.
					->Ayný olan kýsým kelimenin son harfine kadar taranýyor,
					->'\0' karakteri kontrol edilmiyor.
			*/
			while (oneCharFromFile == stringTaken[everyFoundPart]
				&& everyFoundPart < strlen(stringTaken) - 1)
			{
				/*
					Bir sonraki harf aranýyor.
				*/
				fscanf(openedFile, "%c", &oneCharFromFile);
				
				/*
					yeni satýr, tab, boþluk sembolü geldiði zaman;
					Beklenmeyen iþaret deðiþkeni arttýrýlýp,
					sembol yok sayýlýyor.
				*/
				if (oneCharFromFile == spaceSymbol
					|| oneCharFromFile == '\n'
					|| oneCharFromFile == '\t')
					++unexpectedTokens;
				
				/*
					Alýnan harf sonrasý dosya sonuna gelindiyse,
					fonksiyondan çýkýlýyor.
				*/
				else if (feof(openedFile))
					return 0;

				/*
					Bulunan her harf için,
					aranacak kelime içindeki bir sonraki harfe geçiliyor.
				*/
				else
					++everyFoundPart;
			}
			
			/*
				Bulunan harf sayýsý, aranan kelimedeki harf sayýsýna eþitse;
				Kelimenin bulunma sayýsýný 1 arttýrýyoruz,
				Ýçeriði ekrana basýyoruz.
			*/
			if (everyFoundPart == strlen(stringTaken) - 1)
			{
				++foundTimes;
				printf("Times of Found: %d, ", foundTimes);
				printf("Row: %d, ", rowinFile);
				printf("Column: %d.\n", columninFile);
			}

			/*
				Dosya içinde harfler tek tek tarandýktan sonra,
				tamamý bulunsa da bulunmasa da, taranan tüm harflerden 1 azý kadar
				geri gidiliyor.
			*/
			fseek(openedFile, SEEK_CUR, -(everyFoundPart + unexpectedTokens) + 1);
			everyFoundPart = 0;
			unexpectedTokens = 0;
		}
		++columninFile;
	} while (foundTimes == 0);
	return 0;
}