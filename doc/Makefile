all:
	halibut --pdf=radare.pdf radare.but
	halibut --text=radare.pdf.txt radare.but > /dev/null 2>&1
	halibut --html=radare.pdf.html radare.but

push:
	scp radare.pdf radare.pdf.html news.nopcode.org:/home/www/radarenopcode/get
