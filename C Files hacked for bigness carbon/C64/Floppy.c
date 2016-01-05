
#include <stdio.h>
#include <string.h>
#include "nfd.h"

#include "Serial.h"
#include "Floppy.h"
#include "FileTypes.h"
#include "DebugWindow.h"
#include "FileHelpers.h"


#define	InternalError(a)	debug_window_printf("Internal Error");

#define kSerialOK				0
#define kFloppyError			2
#define kSerialFileNotFound		4
#define kSerialError			4
#define kSerialEOF				64

int ExternalRead(unsigned char *buf, int track, int sector);


typedef struct bufferinfo_s {
    int     mode;	/* mode on this buffer */
    int     readmode;	/* is this for reading or writing */
    int     secondary;	/* this is not needed */
    byte   *buffer;	/* use this to save data */
    byte   *slot;	/* save data for directory-slot */
    int     bufptr;	/* use this to save/read data to disk */
    int     length;	/* directory-read length */
}       bufferinfo_t;

//typedef long off_t;


byte normBuffer[16][256];
byte normSlot[16][32];

unsigned char	*dirBuffer;


static int floppyInDrive=0;


#define BUFFER_NOT_IN_USE		0
#define BUFFER_DIRECTORY_READ		1
#define BUFFER_SEQUENTIAL		2
#define BUFFER_MEMORY_BUFFER		3
#define BUFFER_OTHER			4
#define BUFFER_COMMAND_CHANNEL		5


#define WRITE_BLOCK             512


/*
 * Actually, serial-code errors ...
 */

#define SLOT_TYPE_OFFSET		2
#define SLOT_FIRST_TRACK		3
#define SLOT_FIRST_SECTOR		4
#define SLOT_NAME_OFFSET		5
#define SLOT_NR_BLOCKS			30

#define BAM_FIRST_TRACK			0
#define BAM_FIRST_SECTOR		1
#define BAM_FORMAT_TYPE			2
#define BAM_BIT_MAP			4
#define BAM_DISK_NAME			144
#define BAM_DISK_ID			162
#define BAM_VERSION			165


#define BAM_SET(n)    (bamp[1+(n)/8] |= (1 << ((n) % 8)))
#define BAM_CLR(n)    (bamp[1+(n)/8] &= ~(1 << ((n) % 8)))
#define BAM_ISSET(n)  (bamp[1+(n)/8] & (1 << ((n) % 8)))


static byte bam[256];

static int sector_map[36] =
{
    0,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    19, 19, 19, 19, 19, 19, 19,
    18, 18, 18, 18, 18, 18,
    17, 17, 17, 17, 17
};


static char *slot_type[] =
{
    "   ", "SEQ", "PRG", "USR", "REL", "   ", "   ", "   "
};

static bufferinfo_t floppy_buffers[16];

static int floppy_find_slot;
static byte floppy_find_buffer[256];
static int floppy_find_current_track;
static int floppy_find_current_sector;

FILE *     FloppyActiveFILE = NULL;

static char *floppy_find_name;
static int floppy_find_length;
static int floppy_find_type;


int FloppyInitialize ( void );
byte Open1541 ( char *name, int length, int secondary );
byte Close1541 ( int secondary );
byte Read1541 ( unsigned char *data, int secondary );
byte Write1541 ( unsigned char data, int secondary );
void Flush1541 ( int secondary );
static int floppy_parse_name ( char *name, int length, char *realname, int *reallength, int *readmode, int *filetype );
static int floppy_create_directory ( char *name, int length, int filetype, int secondary, unsigned char *outputptr );
static void set_find_first_slot ( char *name, int length, int type );
static unsigned char *find_next_slot ( void );
static void no_a0_pads ( unsigned char *ptr, int l );
static int floppy_read_block ( unsigned char *buf, int track, int sector );
static int floppy_write_block ( unsigned char *buf, int track, int sector );
static void floppy_error ( int code, int track, int sector );
static void remove_slot ( unsigned char *slot );
static int write_sequential_buffer ( unsigned char *slot, unsigned char *buf, int length );
static int alloc_free_sector ( int *track, int *sector );
static int allocate_sector ( int track, int sector );
static off_t offset_from_track_and_sector ( int track, int sector );
static int free_sector ( int track, int sector );
static int floppy_name_match ( unsigned char *slot, char *name, int length, int type );
static void floppy_close_all_channels ( void );
static int do_initialize ( void );
static int do_validate ( void );
static int do_format ( char *name, unsigned char *id, unsigned char *minus );
static int do_command_channel_write ( unsigned char *buf, int length );
static int do_block_command ( char command, char *buffer );
extern void attach_floppy_image ( char *name );
static int mystrncpy ( unsigned char *d, unsigned char *s, int n );
extern int write_fs ( unsigned char data, int secondary );
extern int petconv ( int c );
extern void petconvstring ( char *c );
extern int read_fs ( unsigned char *data, int secondary );
extern int open_fs ( char *name, int length, int secondary );
extern int close_fs ( int secondary );

int PetConv(int c);
void PetConvString(Str32 c);

int PetConv(int c)
{
	switch (c&0xe0)
	{
		case 0x40:
		case 0x60:
			return (c^0x20);
	}
	return (c);
}

void PetConvString(Str32 c)
{
	int i;
	for (i=1; i<c[0]+1; i++) c[i]=PetConv(c[i]);
}

/*
This Function 
*/
int FloppyInitialize(void)
{
	int i;
	
	dirBuffer=(unsigned char *)malloc(8192);
	assert (dirBuffer!=NULL);
	
	for (i=0; i<15; i++) floppy_buffers[i].mode=BUFFER_NOT_IN_USE;

	floppy_buffers[15].mode=BUFFER_COMMAND_CHANNEL;
	floppy_buffers[15].buffer=normBuffer[15];
	
	AddSerialDevice(8, Read1541, Write1541, Open1541, Close1541, Flush1541);

	floppy_error(73, 0, 0);
	
	
	
	return noErr;
}

void	FloppyCleanup (void)
{
	free(dirBuffer);
}

byte Open1541(char *name, int length, int secondary)
{
    bufferinfo_t *p = &floppy_buffers[secondary];
    char    realname[256];
    int     reallength;
    int     readmode;
    int     filetype;

    byte *slot;
    int     i;

    fprintf(stderr,"Open1541 %p\n",FloppyActiveFILE);
    /*
     * No floppy in drive ?
     */
    if (FloppyActiveFILE == NULL)
    	{
            fprintf(stderr," no floppy in drive - let's ask for one\n");
            fflush (stderr);
		//EventRecord			pullKey;
		
		//pull key out of event queue
		//WaitNextEvent(keyDownMask,&pullKey,0,NULL);
    	
    	if (!FloppySelectImage())	//prompt user to insert a disk
			return kFloppyError;
    	if (FloppyActiveFILE ==NULL)		//check to see if there still isn't a valid disk
			return	kFloppyError;	
    	}

    /*
     * Clear error flag
     */
    floppy_error(0, 0, 0);

    /*
     * If channel is command channel, name will be used as write. Return only
     * status of last write ...
     */
    if (p -> mode == BUFFER_COMMAND_CHANNEL) {
	int     i;
	int     status = kSerialOK;
	for (i = 0; i < length; i++)
	    status = Write1541(name[i], secondary);
	return status;
    }
    /*
     * In use ?
     */
    if (p -> mode != BUFFER_NOT_IN_USE) {
	floppy_error(70, 0, 0);
	return kFloppyError;
    }
    /*
     * Filemode / type
     */
    if (secondary == 1)
	readmode = 0;
    else
	readmode = 1;

    filetype = 0;

    i = floppy_parse_name(name, length, realname, &reallength,
	&readmode, &filetype);

    if (i != kSerialOK)
	return i;

    /*
     * Internal buffer ?
     */
    if (*name == '#') {
	p -> mode = BUFFER_MEMORY_BUFFER;
	p -> buffer = normBuffer[secondary];
	p -> bufptr = 0;
	return kSerialOK;
    }
    /*
     * Directory read
     */
    if (*name == '$') {
	if (!readmode)
	    return kFloppyError;

	p -> mode = BUFFER_DIRECTORY_READ;
	p -> buffer = dirBuffer;

	/*
	 * fix this someday
	 */
	if (*realname != '$')
	    p -> length = floppy_create_directory(realname, reallength, filetype,
		secondary, p -> buffer);
	else
	    p -> length = floppy_create_directory("*", 1, 0, secondary,
		p -> buffer);
	p -> bufptr = 0;

	return kSerialOK;
    }
    /*
     * now, set filetype according secondary address, if it was not specified
     * on filename
     */
    if (!filetype)
	filetype = secondary < 2 ? 2 : 1;

    /*
     * Check that there is room on directory.
     */
    set_find_first_slot(name, length, 0);
    slot = find_next_slot();

    p -> readmode = readmode;

    /*
     * Open file for reading
     */
    if (readmode) {
	int     type;
	int     status;

	if (!slot) {
	    Close1541(secondary);
	    floppy_error(62, 0, 0);
	    return kFloppyError;
	}
	type = slot[SLOT_TYPE_OFFSET] & 0x07;

	if (type != filetype) {
	    floppy_error(64, 0, 0);
	    return kFloppyError;
	}
	/*
	 * Seq, Prg
	 */
	if (type == 1 || type == 2) {
	    p -> mode = BUFFER_SEQUENTIAL;
	    p -> bufptr = 2;
	    p -> buffer = normBuffer[secondary];

	    status = floppy_read_block(p -> buffer, (int) slot[SLOT_FIRST_TRACK],
		(int) slot[SLOT_FIRST_SECTOR]);

	    if (status < 0) {
		Close1541(secondary);
		return kFloppyError;
	    }
	    return kSerialOK;
	}
	/*
	 * Unsupported filetype
	 */
	return kFloppyError;
    }
    /*
     * Write
     */
    if (slot) {
	if (*name == '@')
	    remove_slot(slot);
	else {
	    Close1541(secondary);
	    floppy_error(63, 0, 0);
	    return kFloppyError;
	}
    }
    /*
     * Create slot information. (XXXX: should write directoryentry to disk ?)
     */
    p -> slot = normSlot[secondary];
    memset(p -> slot, 0, 32);
    memset(p -> slot + SLOT_NAME_OFFSET, 0xa0, 16);
    memcpy(p -> slot + SLOT_NAME_OFFSET, realname, reallength);

    p -> slot[SLOT_TYPE_OFFSET] = filetype;

    p -> buffer = normBuffer[secondary];
    p -> mode = BUFFER_SEQUENTIAL;
    p -> bufptr = 2;

    return kSerialOK;
}


byte Close1541(int secondary)
{
    byte   *e;
    bufferinfo_t *p = &floppy_buffers[secondary];

    switch (p -> mode) {
      case BUFFER_NOT_IN_USE:
	return kFloppyError;

      case BUFFER_MEMORY_BUFFER:
      case BUFFER_DIRECTORY_READ:
	p -> mode = BUFFER_NOT_IN_USE;
	p -> buffer = NULL;
	break;

      case BUFFER_SEQUENTIAL:
	if (!p -> readmode) {
	    /*
	     * Flush bytes and write slot to directory
	     */
	    write_sequential_buffer(p -> slot, p -> buffer, p -> bufptr);
	    set_find_first_slot(NULL, -1, 0);

	    e = find_next_slot();

	    if (!e) {
		p -> mode = BUFFER_NOT_IN_USE;
		p -> buffer = NULL;

		floppy_error(72, 0, 0);
		return kFloppyError;
	    }
	    p -> slot[SLOT_TYPE_OFFSET] |= 0x80;
	    memcpy(&floppy_find_buffer[floppy_find_slot * 32 + 2], p -> slot + 2,
		30);

	    floppy_write_block(floppy_find_buffer,
		floppy_find_current_track,
		floppy_find_current_sector);
	    floppy_write_block(bam, 18, 0);
	}
	p -> mode = BUFFER_NOT_IN_USE;
	p -> buffer = NULL;
	break;

      case BUFFER_COMMAND_CHANNEL:
	floppy_close_all_channels();
	break;

      default:
      InternalError(0);
    }

    return kSerialOK;
}


byte Read1541(unsigned char *data, int secondary)
{
    bufferinfo_t *p = &floppy_buffers[secondary];

    switch (p -> mode) {
      case BUFFER_NOT_IN_USE:
	floppy_error(61, 0, 0);
	return kFloppyError;

      case BUFFER_DIRECTORY_READ:
	if (p -> bufptr >= p -> length)
	    return kSerialEOF;
	*data = p -> buffer[p -> bufptr];
	p -> bufptr++;
	break;

      case BUFFER_MEMORY_BUFFER:
	if (p -> bufptr >= 256)
	    return kSerialEOF;
	*data = p -> buffer[p -> bufptr];
	p -> bufptr++;
	break;

      case BUFFER_SEQUENTIAL:
	if (!p -> readmode)
	    return kFloppyError;

	/*
	 * Read next block if needed
	 */
	if (p -> buffer[0]) {
	    if (p -> bufptr >= 256) {
		floppy_read_block(p -> buffer,
		    (int) p -> buffer[0],
		    (int) p -> buffer[1]);
		p -> bufptr = 2;
	    }
	} else {
	    if (p -> bufptr > p -> buffer[1])
		return kSerialEOF;
	}

	*data = p -> buffer[p -> bufptr];
	p -> bufptr++;
	break;

      case BUFFER_COMMAND_CHANNEL:
	if (p -> bufptr > p -> length) {
	    floppy_error(0, 0, 0);
	    return kSerialEOF;
	}
	*data = p -> buffer[p -> bufptr];
	p -> bufptr++;
	break;

      default:
		InternalError(0);
    }

    return kSerialOK;
}


byte Write1541(unsigned char data, int secondary)
{
    int     total = 0;
    int     e;
    bufferinfo_t *p = &floppy_buffers[secondary];

    switch (p -> mode) {
      case BUFFER_NOT_IN_USE:
	floppy_error(61, 0, 0);
	return kFloppyError;

      case BUFFER_DIRECTORY_READ:
	floppy_error(60, 0, 0);
	return kFloppyError;

      case BUFFER_MEMORY_BUFFER:
	if (p -> bufptr >= 256)
	    return kFloppyError;
	p -> buffer[p -> bufptr] = data;
	p -> bufptr++;
	return kSerialOK;

      case BUFFER_SEQUENTIAL:
	if (p -> readmode)
	    return kFloppyError;

	if (p -> bufptr >= 256) {
	    p -> bufptr = 2;
	    e = write_sequential_buffer(p -> slot, p -> buffer, WRITE_BLOCK);
	    if (e < 0)
		return kFloppyError;
	}
	p -> buffer[p -> bufptr] = data;
	p -> bufptr++;
	break;

      case BUFFER_COMMAND_CHANNEL:
	if (p -> bufptr >= 256)
	    return kFloppyError;
	p -> buffer[p -> bufptr] = data;
	p -> bufptr++;
	break;

      default:
		InternalError(0);
    }

    return total;
}


void Flush1541(int secondary)
{
    bufferinfo_t *p = &floppy_buffers[secondary];

    if (p -> mode != BUFFER_COMMAND_CHANNEL)
	return;

    do_command_channel_write(p -> buffer, p -> bufptr);
    p -> bufptr = 0;
    floppy_error(0, 0, 0);
}



/*
 * Parse name realname, type and read/write mode. '@' on write must
 * be checked elsewhere
 */
static int floppy_parse_name(char *name, int length, char *realname, int *reallength, int *readmode, int *filetype)
{
    char   *p;
    char   *c;
    int     t;

    p = (char *)memchr(name, ':', length);
    if (p)
	p++;
    else
	p = name;

    if (*name == '@' && p == name)
	p++;

    t = length - (p - name);
    *reallength = 0;
    c = realname;

    while (t > 0 && *p != ',') {
	t--;
	(*reallength)++;
	*c++ = *p++;
    }

    /*
     * Change modes ?
     */
    while (t > 0) {
	t--;
	p++;

	if (t == 0) {
	    return kFloppyError;
	}
	if (*p == 'R')
	    *readmode = 1;
	else if (*p == 'W')
	    *readmode = 0;
	else if (*p == 'S')
	    *filetype = 1;
	else if (*p == 'P')
	    *filetype = 2;
	else if (*p == 'U')
	    *filetype = 3;
	else if (*p == 'R')
	    *filetype = 4;
	else
	    return kFloppyError;

	c = (char *)memchr(p, ',', t);

	if (c) {
	    t -= (c - p);
	    p = c;
	} else
	    t = 0;
    }

    return kSerialOK;
}


#define SET_LO_HI(p, val) {*(p)++ = (val) & 0xff; *(p)++ = ((val)/256) & 0xff;}

/*
 * Create directory listing.
 * If filetype is 0, match for all
 */
static int floppy_create_directory(char *name, int length, int filetype, int secondary, unsigned char *outputptr)
{
    byte   *l;
    byte *p;
    byte   *tmpsl;
    byte   *origptr = outputptr;
    int     t;
    int     bf;

    int     addr;


    set_find_first_slot(name, length, filetype);

    l = outputptr;

    /*
     * Start-address
     */
    addr = 0x401;
    SET_LO_HI(l, addr);

    /*
     * Set next line offset later
     */
    l += 2;

    /*
     * Line number 0
     */
    SET_LO_HI(l, 0);

    /*
     * Reverse on
     */
    *l++ = (char) 0x12;

    *l++ = '\"';
    memcpy(l, &bam[BAM_DISK_NAME], 16);
    no_a0_pads(l, 16);
    l += 16;
    *l++ = '\"';
    *l++ = ' ';
    memcpy(l, &bam[BAM_DISK_ID], 5);
    no_a0_pads(l, 5);
    l += 5;

    *l++ = 0;

    /*
     * Address of next line
     */
    addr += ((l - outputptr) - 2);

    outputptr[2] = addr & 0xff;
    outputptr[3] = (addr / 256) & 0xff;

    outputptr = l;

    /*
     * Now, list files
     */
    while ((p = find_next_slot())) {
	if (p[SLOT_TYPE_OFFSET]) {

	    l += 2;

	    /*
	     * Length and spaces
	     */
	    t = p[SLOT_NR_BLOCKS] + p[SLOT_NR_BLOCKS + 1] * 256;

	    SET_LO_HI(l, t);

	    if (t < 10)
		*l++ = ' ';
	    if (t < 100)
		*l++ = ' ';

	    /*
	     * name
	     */
	    *l++ = ' ';
	    *l++ = '\"';
	    memcpy(l, &p[SLOT_NAME_OFFSET], 16);

	    for (t = 0; (t < 16) && (p[SLOT_NAME_OFFSET + t] != 0xa0);)
		t++;

	    no_a0_pads(l, 16);

	    l[16] = ' ';
	    l[t] = '\"';
	    l += 17;

	    /*
	     * type
	     */
	    *l++ = p[SLOT_TYPE_OFFSET] & 0x80 ? ' ' : '*';

	    tmpsl = (byte *)slot_type[p[SLOT_TYPE_OFFSET] & 0x07];

	    memcpy(l, tmpsl, 3);
	    l += 3;

	    /*
	     * End
	     */
	    *l++ = ' ';
	    *l++ = ' ';
	    *l++ = (char) 0;

	    /*
	     * New address
	     */
	    addr += l - outputptr;

	    outputptr[0] = addr & 0xff;
	    outputptr[1] = (addr / 256) & 0xff;

	    outputptr = l;
	}
    }

    /*
     * calculate blocks free
     */
    bf = 0;

    for (t = 1; t <= 35; t++) {
	if (t != 18)
	    bf += bam[BAM_BIT_MAP + 4 * (t - 1)];
    }

    *l++ = 0;
    *l++ = 0;
    SET_LO_HI(l, bf);
    memcpy(l, "BLOCKS FREE.", 12);
    l += 12;
    *l++ = (char) 0;

    addr += l - outputptr;

    outputptr[0] = addr & 0xff;
    outputptr[1] = (addr / 256) & 0xff;

    /*
     * end
     */
    *l++ = (char) 0;
    *l++ = (char) 0;
    *l++ = (char) 0;

    return l - origptr;
}



/*
 * Initialize slot find
 */
static void set_find_first_slot(char *name, int length, int type)
{
    floppy_find_name = name;
    floppy_find_length = length;
    floppy_find_type = type;

    /*
     * Make sure find_next_slot() loads first directoryblock ...
     */
    floppy_find_slot = 8;
    floppy_read_block(floppy_find_buffer, 18, 0);
}

static byte *find_next_slot(void)
{
    static byte return_slot[32];
    int     status;

    floppy_find_slot++;

    /*
     * Loop all directoryblocks staring from track 18, sector 1
     */
    do {
	/*
	 * Load next(first) directory block ?
	 */
	if (floppy_find_slot >= 8) {
	    if (!(*floppy_find_buffer))
		return NULL;

	    floppy_find_slot = 0;

	    floppy_find_current_track = (int) floppy_find_buffer[0];
	    floppy_find_current_sector = (int) floppy_find_buffer[1];

	    status = floppy_read_block(floppy_find_buffer,
		floppy_find_current_track,
		floppy_find_current_sector);
	}
	while (floppy_find_slot < 8) {
	    if (floppy_name_match(&floppy_find_buffer[floppy_find_slot * 32],
		    floppy_find_name, floppy_find_length,
		    floppy_find_type)) {
		memcpy(return_slot,
		    &floppy_find_buffer[floppy_find_slot * 32], 32);
		return return_slot;
	    }
	    floppy_find_slot++;
	}
    } while (*floppy_find_buffer);

    /*
     * If length < 0, create new directory-entry if possible
     */
    if (floppy_find_length < 0) {
	int     s;

	for (s = 1; s < sector_map[18]; s++) {
	    if (allocate_sector(18, s)) {
		floppy_find_buffer[0] = 18;
		floppy_find_buffer[1] = s;
		floppy_write_block(floppy_find_buffer,
		    floppy_find_current_track,
		    floppy_find_current_sector);
		floppy_find_slot = 0;
		memset(floppy_find_buffer, 0, 256);
		floppy_find_buffer[1] = 0xff;
		floppy_find_current_sector = s;
		return floppy_find_buffer;
	    }
	}

    }
    return NULL;
}


static void no_a0_pads(unsigned char *ptr, int l)
{
    while (l--) {
	if (*ptr == 0xa0)
	    *ptr = 0x20;

	ptr++;
    }
}



/*
 * Read one block
 */
static int floppy_read_block(unsigned char *buf, int track, int sector)
{
	long offset, len;


    
	offset=offset_from_track_and_sector(track, sector);
    if (offset<0) return -1;

    fprintf(stderr,"floppy_read_block track %d sector %d offset %d\n",track,sector,offset);
    
    int result = fseek(FloppyActiveFILE,  offset, SEEK_SET);
    fprintf(stderr,"fseek result %d\n",result);
//	SetFPos(FloppyActiveFd, fsFromStart, offset);
	len=256;
    //FSRead(FloppyActiveFd, &len, buf);
    len = fread(buf,1,len,FloppyActiveFILE);
    fprintf(stderr,"fread %ld\n",len);
	if (len<256)
		{
//
//	we should fix this to cause a commodore error
//
//		DebugStr("\pFloppy read error");
            fprintf(stderr,"Floppy Read Error!\n");
		debug_window_printf("Floppy Read Error!");
		floppy_close_all_channels();
	
		if (floppyInDrive) fclose(FloppyActiveFILE);
		floppyInDrive=0;
		FloppyActiveFILE=NULL;
		return -1;
		
		}
	return 0;
}



/*
 * Write one block
 */
static int floppy_write_block(unsigned char *buf, int track, int sector)
{
	long offset, len;

	offset=offset_from_track_and_sector(track, sector);
	if (offset<0) return -1;

    fseek(FloppyActiveFILE,offset, SEEK_SET);
//	SetFPos(FloppyActiveFd, fsFromStart, offset);
	len=256;
	//FSWrite(FloppyActiveFd, &len, buf);
    len = fwrite(buf,1,len,FloppyActiveFILE);
	if (len<256) debug_window_printf("Floppy write error");
	return 0;
}


/*
 * Error messages
 */
typedef struct errortext_s {
    int     nr;
    char   *text;
}       errortext_t;

static errortext_t floppy_error_messages[] =
{
    {0, "OK"},
    {1, "FILES SCRATCHED"},
    {30, "SYNTAX ERROR"},
    {60, "WRITE FILE OPEN"},
    {61, "FILE NOT OPEN"},
    {62, "FILE NOT FOUND"},
    {63, "FILE EXISTS"},
    {64, "FILE TYPE MISMATCH"},
    {70, "NO CHANNEL"},
    {72, "DISK FULL"},
    {73, "TVR 1541 EMULATOR V1.0"},
    {-1, 0},
};


/*
 * Should set values to somewhere so that they could be read from
 * command channel
 */
static void floppy_error(int code, int track, int sector)
{
    char   *message;
    bufferinfo_t *p = &floppy_buffers[15];

    errortext_t *e = &floppy_error_messages[0];

    while (e -> nr >= 0 && e -> nr != code)
	e++;

    if (e -> nr >= 0)
	message = e -> text;
    else
	message = "UNKNOWN ERROR NUMBER";

    sprintf((char *)p -> buffer, "%02d, %s,%d,%d", code, message, track, sector);
    p -> length = strlen((char *)p -> buffer);
    p -> bufptr = 0;
}


static void remove_slot(unsigned char *slot)
{
    byte    buf[256];
    int     tmp;
    int     t, s;

    /*
     * Find slot
     */
    for (tmp = 0; (tmp < 16) && slot[SLOT_NAME_OFFSET + tmp] != 0xa0; tmp++);

    set_find_first_slot((char *)&slot[SLOT_NAME_OFFSET], tmp,
	slot[SLOT_TYPE_OFFSET] & 0x07);

    /*
     * If slot slot found, remove
     */
    if (find_next_slot()) {
	/*
	 * Free all buffers from bam
	 */
	t = (int) floppy_find_buffer[floppy_find_slot * 32 + SLOT_FIRST_TRACK];
	s = (int) floppy_find_buffer[floppy_find_slot * 32 + SLOT_FIRST_SECTOR];

	while (t) {
	    free_sector(t, s);
	    floppy_read_block(buf, t, s);
	    t = (int) buf[0];
	    s = (int) buf[1];
	}

	/*
	 * Update bam
	 */
	floppy_write_block(bam, 18, 0);

	/*
	 * Update directory entry
	 */
	floppy_find_buffer[floppy_find_slot * 32 + SLOT_TYPE_OFFSET] = 0;
	floppy_write_block(floppy_find_buffer,
	    floppy_find_current_track,
	    floppy_find_current_sector);
    }
}



static int write_sequential_buffer(unsigned char *slot, unsigned char *buf, int length)
{
    static int t = 0, s;
    int     t_new, s_new;
    int     e;

    /*
     * First block of a file ?
     */
    if (t == 0) {
	e = alloc_free_sector(&t, &s);
	if (e < 0) {
	    floppy_error(72, 0, 0);
	    return -1;
	}
	slot[SLOT_FIRST_TRACK] = t;
	slot[SLOT_FIRST_SECTOR] = s;
    }
    if (length == WRITE_BLOCK) {
	/*
	 * Write current sector and allocate next
	 */
	e = alloc_free_sector(&t_new, &s_new);
	if (e < 0) {
	    floppy_error(72, 0, 0);
	    return -1;
	}
	buf[0] = t_new;
	buf[1] = s_new;

	floppy_write_block(buf, t, s);

	t = t_new;
	s = s_new;

	if (!(++slot[SLOT_NR_BLOCKS]))
	    ++slot[SLOT_NR_BLOCKS + 1];

    } else {
	/*
	 * Write last block
	 */
	buf[0] = 0;
	buf[1] = length - 1;

	floppy_write_block(buf, t, s);

	t = 0;

	if (!(++slot[SLOT_NR_BLOCKS]))
	    ++slot[SLOT_NR_BLOCKS + 1];
    }
    return 0;
}



static int alloc_free_sector(int *track, int *sector)
{
    int     t, s;

    for (t = 17; t >= 1; t--) {
	for (s = 0; s < sector_map[t]; s++) {
	    if (allocate_sector(t, s)) {
		*track = t;
		*sector = s;
		return 0;
	    }
	}
    }

    for (t = 19; t <= 35; t++) {
	for (s = 0; s < sector_map[t]; s++) {
	    if (allocate_sector(t, s)) {
		*track = t;
		*sector = s;
		return 0;
	    }
	}
    }
    return -1;
}


static int allocate_sector(int track, int sector)
{
    /*
     * Macros use this
     */
    byte   *bamp;

    bamp = &bam[BAM_BIT_MAP + 4 * (track - 1)];

    if (BAM_ISSET(sector)) {
	(*bamp)--;
	BAM_CLR(sector);

	return 1;
    }
    return 0;
}


/*
 * Return block offset on 'disk file' according to track and sector
 */
static off_t offset_from_track_and_sector(int track, int sector)
{
    int     sectors, i;

    if ((track < 1) || (track > 35))
	return (-1);

    for (sectors = 0, i = 1; i < track; i++) {
	sectors += sector_map[i];
    }

    if ((sector < 0) || (sector >= sector_map[track]))
	return (-1);

    sectors += sector;

    return 256L * sectors;
}



static int free_sector(int track, int sector)
{
    byte   *bamp;

    bamp = &bam[BAM_BIT_MAP + 4 * (track - 1)];

    if (!(BAM_ISSET(sector))) {
	BAM_SET(sector);
	(*bamp)++;
	return 1;
    }
    return 0;
}


/*
 * If type is 0, match for next slot with same name
 * If type is nonzero, match next that has the same id and name
 */
static int floppy_name_match(unsigned char *slot, char *name, int length, int type)
{
    int     i;
    /* int comma = 0; */

    if (length < 0) {
	if (slot[SLOT_TYPE_OFFSET])
	    return 0;
	else
	    return 1;
    }
    if (!slot[SLOT_TYPE_OFFSET])
	return 0;

    if (length >= 16)
	length = 16;

    for (i = 0; i < length; i++) {
	switch (name[i]) {
	  case '?':
	    /*
	     * Match any character
	     */
	    break;

	  case '*':
	    /*
	     * Make a match
	     */
	    i = 16;
	    break;

	  default:
	    if ((byte)name[i] != slot[i + SLOT_NAME_OFFSET])
		return 0;
	}
    }

    /*
     * Got name match ?
     */
    if (i < 16 && slot[SLOT_NAME_OFFSET + i] != 0xa0)
	return 0;

    if (type && type != (slot[SLOT_TYPE_OFFSET] & 0x07))
	return 0;

    return 1;
}



/*
 * Close all channels. This happens on 'I' -command and on command-
 * channel close
 */
void floppy_close_all_channels(void)
{
    int     i;
    bufferinfo_t *p;

    for (i = 0; i <= 15; i++) {
	p = &floppy_buffers[i];

	if (p -> mode != BUFFER_NOT_IN_USE && p -> mode != BUFFER_COMMAND_CHANNEL)
	    Close1541(i);
    }
}


static int do_initialize(void)
{

    floppy_close_all_channels();
    return kSerialOK;
}

static int do_validate(void)
{
    int     t, s;
    byte *b;
    byte    tmp[256];

    do_initialize();

    b = (byte *) bam + BAM_BIT_MAP;

    for (t = 1; t <= 35; t++) {
	*b++ = 0;
	*b++ = 0;
	*b++ = 0;
	*b++ = 0;

	for (s = 0; s < sector_map[t]; s++)
	    free_sector(t, s);
    }

    set_find_first_slot("*", 1, 0);
/*
	First map out the directory itself.	-Dan Miner
	And another one bites the dust....  :)
*/
	t = 18; s = 0;
	while(t) {
		allocate_sector(t, s);
		floppy_read_block(tmp, t, s);
		t = (int) tmp[0];
		s = (int) tmp[1];
	}

    while ((b = (byte *) find_next_slot())) {
	if (b[SLOT_TYPE_OFFSET]) {
	    floppy_find_buffer[floppy_find_slot * 32 + SLOT_TYPE_OFFSET] |= 0x80;
	    floppy_write_block(floppy_find_buffer, floppy_find_current_track,
		floppy_find_current_sector);
	    t = (int) b[SLOT_FIRST_TRACK];
	    s = (int) b[SLOT_FIRST_SECTOR];

	    while (t) {
		allocate_sector(t, s);
		floppy_read_block(tmp, t, s);
		t = (int) tmp[0];
		s = (int) tmp[1];
	    }
	}
    }

    floppy_write_block(bam, 18, 0);
    return kSerialOK;
}


static int do_format(char *name, unsigned char *id, unsigned char *minus)
{
    byte    tmp[256];
    int     status;

    byte    null = 0;

    if (!name) {
	floppy_error(30, 0, 0);
	return kFloppyError;
    }
    /*
     * If id, skip comma
     */
    if (id)
	*id++ = 0;
    else
	id = &null;

    /*
     * Make dir-entry
     */
    memset(tmp, 0, 256);
    tmp[1] = 255;
    floppy_write_block(tmp, 18, 1);

    /*
     * Make bam
     */
    memset(bam, 0, 256);
    bam[0] = 18;
    bam[1] = 1;
    bam[2] = 65;
    memset(bam + BAM_DISK_NAME, 0xa0, 16);
    mystrncpy(bam + BAM_DISK_NAME, (byte *)name + 1, 16);

    memset(bam + BAM_DISK_ID, 0xa0, 5);
    mystrncpy(bam + BAM_DISK_ID, id, 2);
    bam[BAM_VERSION] = 50;
    bam[BAM_VERSION + 1] = 65;
    status = do_validate();
    allocate_sector(18, 1);		/* first directory block-  Dan Miner */
    floppy_write_block(bam, 18, 0);
    return status;
}


static int do_command_channel_write(unsigned char *buf, int length)
{
    int     status = kFloppyError;
    static byte p[256];
    char   *name;
    byte   *minus;
    byte   *id;

    memcpy(p, buf, length);
    p[length] = 0;

    name = (char *)memchr(p, ':', length);
    id = (byte *)memchr(p, ',', length);
    minus = (byte *)memchr(p, '-', length);

    floppy_error(0, 0, 0);

    switch (p[0]) {
      case 'N':
	status = do_format(name, id, minus);
	break;
      case 'C':
	break;

      case 'R':
	break;

      case 'S':
	break;

      case 'I':
	status = do_initialize();
	break;

      case 'V':
	status = do_validate();
	break;

      case 'B':
	if (!minus || !name)
	    floppy_error(30, 0, 0);
	else
	    status = do_block_command(minus[1], name + 1);
	break;

      case 'U':
	switch (p[1]) {
	  case '1':
	  case 'A':
	    if (name)
		status = do_block_command('R', name + 1);
	    break;

	  case '2':
	  case 'B':
	    if (name)
		status = do_block_command('W', name + 1);
	    break;
	}
	break;

    }

    return status;
}



static int do_block_command(char command, char *buffer)
{
    int     l;

    int     channel = 0;
    int     drive = 0;
    int     track = 0;
    int     sector = 0;
    int     position = 0;

    switch (command) {
      case 'R':
      case 'W':
	l = sscanf(buffer, "%d %d %d %d", &channel, &drive,
	    &track, &sector);
	if (l < 4) {
	    l = sscanf(buffer, "%d,%d,%d,%d", &channel, &drive,
		&track, &sector);
	    if (l < 4) {
		floppy_error(30, 0, 0);
		return kFloppyError;
	    }
	}
	if (floppy_buffers[channel].mode != BUFFER_MEMORY_BUFFER) {
	    floppy_error(30, 0, 0);
	    return kFloppyError;
	}
	if (command == 'W')
	    floppy_write_block(floppy_buffers[channel].buffer,
		track, sector);
	else
	    floppy_read_block(floppy_buffers[channel].buffer,
		track, sector);

	floppy_buffers[channel].bufptr = 0;
	break;

      case 'A':
      case 'F':
	l = sscanf(buffer, "%d %d %d", &drive, &track, &sector);

	if (l < 3) {
	    l = sscanf(buffer, "%d,%d,%d", &drive, &track, &sector);

	    if (l < 3) {
		floppy_error(30, 0, 0);
		return kFloppyError;
	    }
	}
	if (command == 'A')
	    allocate_sector(track, sector);
	else
	    free_sector(track, sector);
	break;

      case 'P':
	l = sscanf(buffer, "%d %d", &channel, &position);

	if (l < 2) {
	    l = sscanf(buffer, "%d,%d", &channel, &position);

	    if (l < 2) {
		floppy_error(30, 0, 0);
		return kFloppyError;
	    }
	}
	floppy_buffers[channel].bufptr = position;
	break;

      default:
	floppy_error(30, 0, 0);
	return kFloppyError;
    }
    return kSerialOK;
}



void AttachFloppyImage(nfdchar_t *spec)
{
    fprintf(stderr,"AttachFloppyImage\n");
	floppy_close_all_channels();

	if (floppyInDrive) fclose(FloppyActiveFILE);
	floppyInDrive=0;
    FloppyActiveFILE=NULL;

	if (spec==NULL)
	{
		floppyInDrive=0;
		return;
	}
	if ((FloppyActiveFILE = fopen(spec, "rw"))==NULL)
	{
		floppyInDrive=0;
		return;
	}

	floppyInDrive=1;
	floppy_read_block(bam, 18, 0);
}

static int mystrncpy(unsigned char *d, unsigned char *s, int n)
{
	while (n-- && *s)
	*d++=*s++;
	return (n);
}

void CreateImage(nfdchar_t *spec)
{
	byte block[256];
	int	i;
	long len;
	//short fnum;
    FILE *theFile= NULL;
	
	for (i=0; i<256; i++) block[i]=0;

    theFile = fopen(spec,"rw");
    
	//FSpCreate(spec, (OSType)APPLTYPE, (OSType)DISKFTYPE, 0);
	//FSpOpenDF(spec, fsRdWrPerm, &fnum);
	
	len=256;
    for (i=0; i < 683; i++) fwrite(block,1,len,theFile);
//    for (i=0; i < 683; i++) FSWrite(fnum, &len, block);
	//FSClose(fnum);
    fclose(theFile);
}


void FloppyCreateImage(void)
{
//	StandardFileReply reply;
//   FSSpec                     chosenFile;

	static char *initString="N0:MAC64 FLOPPY,00";
	
    nfdchar_t *savePath = NULL;
    nfdresult_t result = NFD_SaveDialog( "D64", NULL, &savePath );

    
//	OSErr err=SimpleNavPutFile(kNavGenericSignature,kNavGenericSignature, &chosenFile);
//	StandardPutFile("\pCreate New Image:", "\pDISK.D64", &reply);
	
//	if (reply.sfGood)
//if(err == noErr)
    if ( result == NFD_OKAY )
	{
		
		CreateImage(savePath);
		AttachFloppyImage(savePath);
		do_command_channel_write((byte *)initString, strlen(initString));

	}
	
}

Boolean FloppySelectImage(void)
{
    fprintf(stderr,"FloppySelectImage\n");
    fflush (stderr);

//	FSSpec theFileSpec;
//	StandardFileReply reply;
//	SFTypeList			types;
	//OSErr err=SimpleNavGetFile(&theFileSpec);
    nfdchar_t *outPath = NULL;
    nfdresult_t result = NFD_OpenDialog( "D64", NULL, &outPath );

	//StandardGetFile(nil, (short)-1, types, &reply);
	//if (reply.sfGood)
	//if (err==noErr)
    if ( result == NFD_OKAY )
	{
		AttachFloppyImage(outPath);
		do_initialize();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
	
}

void FloppyCopyTo(void)
{
    // AJS fix this
    fprintf(stderr,"***** FloppyCopyTo\n");
#if 0
//	StandardFileReply reply;
	short fNum;
	long size;
	byte data;
	//SFTypeList			types;
	FSSpec theFileSpec;
	
	if (floppyInDrive==0)
	{
		debug_window_printf("No Floppy Image");
		return;
	}

    nfdchar_t *outPath = NULL;
    nfdresult_t result = NFD_OpenDialog( "t64", NULL, &outPath );

	OSErr err=SimpleNavGetFile(&theFileSpec);
	//StandardGetFile(nil, (short)-1, types, &reply);
	if (err=noErr)
	{

		FSpOpenDF(&theFileSpec, fsRdPerm, &fNum);
		PetConvString(theFileSpec.name);
		Open1541((char *)(&theFileSpec.name[1]), (int)theFileSpec.name[0], 1);
		size=1;
		while (size)
		{
			FSRead(fNum, &size, &data);
			if (size) Write1541(data, 1);
		}
		FSClose(fNum);
		Close1541(1);
	}
#endif
}

int ExternalRead(unsigned char *buf, int track, int sector)
{
	return floppy_read_block(buf, track, sector);
}
