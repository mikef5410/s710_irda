/* Dump data from Polar S725x via IrDA
 * by Vidar 'koala_man' Holen
 * 2008-01-14 
 *
 * Version 0.0.5
 *
 *
 * Provides dumping of heart rate graphs, but not much else.
 *
 * Usage:
 * s725xdump > file     #or if using gnuplot: 
 * s725xdump | gnuplot
 *
 * Make sure the irda0 interface is up. When s725xdump is discovering devices,
 * press 'down' key on the watch to go to "Connect", but DON'T hit 'start'. 
 *
 * Then point the device towards the IrDA receiver, and hopefully all will be
 * well. The output from this app can be fed into gnuplot.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/irda.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

/* Print some extra debug information */
#define DEBUG 1

/* Add gnuplot headers and titles */
#define GNUPLOT 1

/* Ripped from an example by Dag Bratteli. 
 * Find and return the first IRDA device.
 */
#define MAX_DEVICES 10
int discover_devices(int fd)
{
    struct irda_device_list *list;
    unsigned char buf[sizeof(struct irda_device_list) +
                      sizeof(struct irda_device_info) * MAX_DEVICES];
    unsigned int len;
    int daddr;
    int i;

    len = sizeof(struct irda_device_list) +
          sizeof(struct irda_device_info) * MAX_DEVICES;
    list = (struct irda_device_list *) buf;
        

	fprintf(stderr, "Doing device discovery");
	i=60;
    while (getsockopt(fd, SOL_IRLMP, IRLMP_ENUMDEVICES, buf, &len)) {
		if(errno!=EAGAIN) {
			perror("getsockopt");
			return -1;
		}
		i--;

		if(i==0) {
			fprintf(stderr, "\nDidn't find any devices.\n");
			return -1;
		}

		fprintf(stderr,".");
		fflush(stderr);
		sleep(1);
    }
    if (len > 0) {
        /* 
         * Just pick the first one, but we should really ask the 
         * user 
         */
        daddr = list->dev[0].daddr;

        fprintf(stderr,"\nDiscovered: (list len=%d)\n", list->len);

        for (i=0;i<list->len;i++) {
            fprintf(stderr,"  name:  %s\n", list->dev[i].info);
            fprintf(stderr,"  daddr: %08x\n", list->dev[i].daddr);
            fprintf(stderr,"  saddr: %08x\n", list->dev[i].saddr);
            fprintf(stderr,"\n");
        }
		if(i>1) fprintf(stderr, "Picking the first one.\n");
    }
    return daddr;
}

int min(int a, int b) { 
	return a<b?a:b;
}

/* Transmit and print some bytes */
void transmit(int fd, char* buf, int size) {
    int actual = send(fd, buf, size, 0); 
	int i;
	sleep(1);
	printf("Send %02d: ", actual);
	for(i=0; i<actual; i++) {
		printf("%02hhx ", buf[i]);
	}
	printf("\n");
}
/* Receive and print some bytes */
void receive(int fd) {
	char buf[1024];
    int actual = recv(fd, buf, 1024, 0); 
	int i;
	printf("Recv %02d: ", actual);
	for(i=0; i<actual; i++) {
		printf("%02hhx ", buf[i]);
	}
	printf("\n");
}

/* Reset to factory defaults */
void resetdevice(int fd) {
	transmit(fd, "\x09", 1);
}

/* Get and print the device's current time */
void printtime(int fd) { 
	char buf[32];

	buf[0]=2;

	send(fd, buf, 1, 0);
	recv(fd, buf, 32, 0);
	if(buf[0] != 2) {
		fprintf(stderr,"Eeek, wrong packet");
		return;
	}
	fprintf(stderr,"Device time is %02x:%02x:%02x, %02x/%02x 20%02x\n", buf[3], buf[2], buf[1], buf[4], buf[6]&0x0F, buf[5]);
}

/* Get the number of files */
int getfiles(int fd) {
	char buf[32];
	int upper, lower;

	buf[0]=0x15;
	send(fd, buf, 1, 0);
	recv(fd, buf, 32, 0);
	// Now buf[3] has BCD file numbers
	upper = buf[3] & 0xf0;
	upper = upper >> 4;
	lower = buf[3] & 0x0f;
	return ( (upper * 10) + lower );
}

/* Download the files and return the fd to the temp file */
int dlfiles(int fd) {
	char* fn;
	int file;
	char buf[1024];
	int w;

	fn=(char*)malloc(32);
	strcpy(fn, "polarXXXXXX");
	file=mkstemp(fn);
	if(file<0) {
		perror("mkstemp");
		return -1;
	}

#ifndef DEBUG
	unlink(fn);
#else
	fprintf(stderr, "Writing to %s\n", fn);
#endif

	send(fd, "\x0B", 1, 0);
	w=recv(fd, buf, 1024, 0);
	if(w>0) {
		write(file, buf+3, w-3);
	}
	do {
		send(fd, "\x16\x2F", 2, 0);
		w=recv(fd, buf, 1024, 0);
		if(w>0 && buf[0] == 0x0B) {
			fprintf(stderr, "In %d   \r", (((unsigned char)buf[1])<<8)|(unsigned char)buf[2]);
			fflush(stderr);
			write(file, buf+3, w-3);
		} else break;
	} while(1);
	fprintf(stderr,"Done downloading\n");

	lseek(file, 0, SEEK_SET);

	return file;
}

int readshort(int fd) {
	unsigned char buf[2];
	if(read(fd, buf, 2) <= 0) {
		perror("read from file");
		return -1;
	}
	return ((buf[0] << 8) | buf[1]);
}

int skipbytes(int fd, int skip) { 
	char buf[32];
	int r;
	while(skip>0) { 
		r=read(fd, buf, min(32, skip));
		if(r<=0) return 0;
		skip-=r;
	}
	return 1;
}

int flipshort(int s) {
	return ((s&0xFF)<<8) | ((s>>8)&0xFF);
}

int parsefiles(int fd) {
	char buf[1024];
	char title[64];
	int i, size, totalsize, laps, samplerate, ms, lastlap, filenum;
	int manualLaps, units;
	int flagSpeed1, flagSpeed2, flagPower, flagCadence, flagAltitude;
	int samples;
	int* laptimes=NULL;
	int sampleSize, lapSize;

	filenum=0;
	totalsize=readshort(fd);
	readshort(fd); /* unknown number, 28 08 */
#ifdef DEBUG
	fprintf(stdout, "## Processing %d bytes...\n", i);
#endif

#ifdef GNUPLOT
	fprintf(stdout, "## Begin gnuplot header\n");
	fprintf(stdout, "set term png\n");
	fprintf(stdout, "## End gnuplot header\n\n");
#endif

	while(totalsize>0) {
		size=(flipshort(readshort(fd)));
#ifdef DEBUG
		fprintf(stdout, "## Record has %d bytes:\n", size);
#endif
		totalsize-=size;
		size-=2;

		skipbytes(fd, 8); size-=8;
		read(fd, buf, 9); size-=9;
		/* Yes, this is supposed to be hex with one decimal: */
		fprintf(stdout, "# Date: 20%02x-%02d-%02x %02x:%02x:%02x\n", 
				buf[4], buf[5]&0x0F, buf[3], buf[2], buf[1], buf[0]);

		snprintf(title, 512, "20%02x-%02d-%02x %02x:%02x ",
				buf[4], buf[5]&0x0F, buf[3], buf[2], buf[1]);

		fprintf(stdout, "# Duration: %02x:%02x:%02x.%01x\n",
				buf[8], buf[7], buf[6], ((unsigned char)buf[5])>>4);

		read(fd, buf, 2); size-=2;
		fprintf(stdout, "# AvgHR: %d\n", (unsigned char)buf[0]);
		fprintf(stdout, "# MaxHR: %d\n", (unsigned char)buf[1]);

		read(fd, buf, 1); size-=1;
		laps=(unsigned char)buf[0];
		fprintf(stdout, "# Laps: %d\n", laps);
		read(fd, buf, 1); size-=1;
		manualLaps=(unsigned char)buf[0];
		laptimes=calloc(laps+2, sizeof(int));
		laptimes[0]=0;
		laptimes[laps+1]=INT_MAX;
		lastlap=0;

		// Exercise mode and User ID
		read(fd, buf, 2); size-=2;

		// Units
		read(fd, buf, 1); size-=1;
		units=(unsigned char)buf[0];

		// Various optional recordings
		read(fd, buf, 1); size-=1;
		flagSpeed1=(unsigned char)buf[0] & 0x20;
#ifdef DEBUG
		fprintf(stdout, "# Speed (Bike 1): %d\n", flagSpeed1);
#endif
		flagSpeed2=(unsigned char)buf[0] & 0x10;
#ifdef DEBUG
		fprintf(stdout, "# Speed (Bike 2): %d\n", flagSpeed2);
#endif
		flagPower=(unsigned char)buf[0] & 0x08;
#ifdef DEBUG
		fprintf(stdout, "# Speed Power: %d\n", flagPower);
#endif
		flagCadence=(unsigned char)buf[0] & 0x04;
#ifdef DEBUG
		fprintf(stdout, "# Cadence: %d\n", flagCadence);
#endif
		flagAltitude=(unsigned char)buf[0] & 0x02;
#ifdef DEBUG
		fprintf(stdout, "# Altitude: %d\n", flagAltitude);
#endif
		lapSize = 6;
		if (flagAltitude != 0) { lapSize += 5; }
		if ((flagSpeed1 != 0) || (flagSpeed2 != 0)) {
			if (flagCadence != 0) lapSize += 1;
			if (flagPower != 0) lapSize += 4;
			lapSize += 4; // Speed data
			// Interval Training can affect lap size
		}
                sampleSize = 1;
		if (flagAltitude != 0) sampleSize += 2;
		if ((flagSpeed1 != 0) || (flagSpeed2 != 0)) {
			if (flagAltitude != 0) sampleSize -= 1;
			if (flagPower != 0) sampleSize += 4;
			if (flagCadence != 0) sampleSize += 1;
		}

		read(fd, buf, 1); size-=1;
                // & 0xf seems accurate.  The upper bits are used for something else
		switch(buf[0] & 0xf) {
			case 0: samplerate=5; break;
			case 1: samplerate=15; break;
			case 2: samplerate=60; break;
			default: samplerate=1; break;
		}
		if(samplerate==1) 
			fprintf(stdout, "# RecordingRate: unknown (%02hhx)\n", buf[0]);
		else 
			fprintf(stdout, "# RecordingRate: %d\n", samplerate);

		skipbytes(fd, 37); size-=37; /* settings and gibberish */
		skipbytes(fd, 5); size-=5; /* best lap time */
		skipbytes(fd, 3); size-=3; 
		fprintf(stdout, "# Kcal: %x\n", flipshort(readshort(fd))); size-=2;

		skipbytes(fd, 34); size-=34; 
		skipbytes(fd, 21); size-=21; 

		for(i=0; i<laps; i++) {
			read(fd, buf, lapSize); size-=lapSize;
			ms=0;
			ms+=((buf[0]>>6)&0x03) * 100;
			ms+=((buf[1]>>6)&0x03) * 400;
			ms+=((buf[0]   )&0x3F) * 1000;
			ms+=((buf[1]   )&0x3F) * 60000;
			ms+=((buf[2]   )&0x3F) * 3600000;
			laptimes[i+1]=ms;
			ms-=lastlap;
			fprintf(stdout, "# Lap %d: %d:%02d:%02d.%d (%d at %d)\n", 
					i+1, 
					ms/3600000,  
					(ms/60000)%60,
					(ms/1000 )%60,
					(ms/100)%10,
					ms,
					laptimes[i]
					);
			lastlap=laptimes[i+1];
		}

#ifdef GNUPLOT
		fprintf(stdout, "##Begin gnuplot section\n");
		fprintf(stdout, "set output \"output%02d.png\"\n",filenum);
		fprintf(stdout, "set xrange [0:%f]\n", laptimes[laps]/1000.0);
		fprintf(stdout, "set title \"%s --  %d laps\"\n", title, laps);
		fprintf(stdout, "set xlabel \"Seconds\"\n");
		fprintf(stdout, "set ylabel \"BPM\"\n");
		fprintf(stdout, "set style rect fc lt -1 fs solid 0.15 noborder\n");
		fprintf(stdout, "unset obj\n");
		for(i=1; i<laps; i+=2) {
			fprintf(stdout, "set obj rect from %f, graph 0 to %f, graph 1\n",laptimes[i]/1000.0, laptimes[i+1]/1000.0);
		}
		fprintf(stdout, "plot \"-\" using 1:2 title \"Heart Rate\" with lines\n");
		fprintf(stdout, "##End gnuplot section\n");
#endif

		if(size<0) {
			fprintf(stderr, "Parsing error, overshot size: %d\n", size);
			return -1;
		}
		if((size % sampleSize) != 0) {
			fprintf(stderr, "Parsing error, bytes mod sampleSize !=0: %d (trying anyways)\n",size);
		}
		samples=size / sampleSize;

		i=0;
		fprintf(stdout, "## Second   Heartrate ");
		if (flagAltitude != 0) fprintf(stdout, "Altitude ");
		fprintf(stdout, "\n");
		while(size > 0) {
			read(fd, buf, sampleSize); size -= sampleSize;

			samples--;
			fprintf(stdout, "%d   %hhu ", samples*samplerate, buf[0]);
			if (flagAltitude != 0)
				fprintf(stdout, "%d ", ( (int)(buf[2] & 0x1f) << 8 ) + buf[1]);
			fprintf(stdout, "\n");
			i++;
		}

#ifdef DEBUG
		fprintf(stdout, "## Record processed. Bytes remaining: %d\n", totalsize);
#endif
#ifdef GNUPLOT
		fprintf(stdout, "e\n");
#endif
		fprintf(stdout, "## END FILE\n\n");

		filenum++;
	}

	return 1;

}

int main(int argc, char** argv) {
    struct sockaddr_irda peer;
    int daddr;
	int fd;
	int files;


//	parsefiles(0); return 0;

    fd = socket(AF_IRDA, SOCK_STREAM, 0);

    daddr = discover_devices(fd);
	if(daddr == -1) return 1;

    peer.sir_family = AF_IRDA;
    peer.sir_lsap_sel = LSAP_ANY;
    peer.sir_addr = daddr;
    strcpy(peer.sir_name, "HRM");
        
    if(connect(fd, (struct sockaddr *) &peer, sizeof(struct sockaddr_irda))) {
		perror("connect");
		return 1;
	}
	fprintf(stderr, "Connected!\n");

	printtime(fd);
        // I need the number of files.
	files=getfiles(fd);
	fprintf(stderr, "There are %d files\n", files);
	files=dlfiles(fd);
	parsefiles(files);

	close(fd);
	return 0;
}
