@interface NSMutableData( BLPrivate )

- (void) swapBytes:(unsigned) inBytesPerWord;

@end


#pragma mark -


@implementation NSMutableData( BLPrivate )


- (void) swapBytes:(unsigned) inBytesPerWord {
	int						i, n, o;
	UInt8				   *u8;
	UInt16				   *u16;
	UInt32				   *u32a, *u32b;
	register UInt8			u8tmp;
	register UInt16			u16tmp;
	register UInt32			u32tmp, u32atmp, u32btmp;
	
	switch ( inBytesPerWord ) {
		case 2: case 3: case 4: case 8:		break;
		default:							return;
	}

	n = [self length];
	
	if ( ( o = n % inBytesPerWord ) ) {
		o = inBytesPerWord - o;
		[self increaseLengthBy: o];
		n += o;
	}

	if ( inBytesPerWord == 2 ) {
		u16 = (UInt16 *) [self mutableBytes];

		for ( i = 0; i < n; i += inBytesPerWord, ++u16 ) {
			u16tmp = *u16;
			*u16 = u16tmp << 8 | u16tmp >> 8 & 0xff;
		}
	} else if ( inBytesPerWord == 3 ) {
		u8 = (UInt8 *) [self mutableBytes];

		for ( i = 0; i < n; i += inBytesPerWord, u8 += 3 ) {
			u8tmp = u8[ 0 ];
			
			u8[ 0 ] = u8[ 2 ];
			u8[ 2 ] = u8tmp;
		}
	} else if ( inBytesPerWord == 4 ) {
		u32a = (UInt32 *) [self mutableBytes];
		
		for ( i = 0; i < n; i += inBytesPerWord, ++u32a ) {
			u32atmp = *u32a;
			
			*u32a = u32atmp << 24 | ( u32atmp & 0x0ff00 ) << 8 | ( u32atmp & 0x0ff0000 ) >> 8 | ( u32atmp & 0xff000000 ) >> 24;
		}
	} else {
		u32a = (UInt32 *) [self mutableBytes];
		u32b = u32a + 1;
		
		for ( i = 0; i < n; i += inBytesPerWord, u32a += 2, u32b += 2 ) {
			u32atmp = *u32a;
			u32btmp = *u32b;
		
			u32tmp = u32btmp << 24 | ( u32btmp & 0x0ff00 ) << 8 | ( u32btmp & 0x0ff0000 ) >> 8 | ( u32btmp & 0xff000000 ) >> 24;
			*u32b = u32atmp << 24 | ( u32atmp & 0x0ff00 ) << 8 | ( u32atmp & 0x0ff0000 ) >> 8 | ( u32atmp & 0xff000000 ) >> 24;
			*u32a = u32tmp;
		}
	}
}


@end
