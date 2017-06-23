/*
 * options.c
 *
 * Copyright (c) 2001-2002  Ben Fennema <bfennema@falcon.csc.calpoly.edu>
 * Copyright (c) 2014       Pali Rohár <pali.rohar@gmail.com>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/**
 * @file
 * mkudffs option handling functions
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <malloc.h>
#include <ctype.h>
#include <errno.h>

#include "mkudffs.h"
#include "defaults.h"
#include "options.h"

struct option long_options[] = {
	{ "help", no_argument, NULL, OPT_HELP },
	{ "label", required_argument, NULL, OPT_LABEL },
	{ "uuid", required_argument, NULL, OPT_UUID },
	{ "blocksize", required_argument, NULL, OPT_BLK_SIZE },
	{ "udfrev", required_argument, NULL, OPT_UDF_REV },
	{ "lvid", required_argument, NULL, OPT_LVID },
	{ "vid", required_argument, NULL, OPT_VID },
	{ "vsid", required_argument, NULL, OPT_VSID },
	{ "fsid", required_argument, NULL, OPT_FSID },
	{ "fullvsid", required_argument, NULL, OPT_FULLVSID },
	{ "uid", required_argument, NULL, OPT_UID },
	{ "gid", required_argument, NULL, OPT_GID },
	{ "bootarea", required_argument, NULL, OPT_BOOTAREA },
	{ "strategy", required_argument, NULL, OPT_STRATEGY },
	{ "spartable", required_argument, NULL, OPT_SPARTABLE },
	{ "packetlen", required_argument, NULL, OPT_PACKETLEN },
	{ "media-type", required_argument, NULL, OPT_MEDIA_TYPE },
	{ "space", required_argument, NULL, OPT_SPACE },
	{ "ad", required_argument, NULL, OPT_AD },
	{ "noefe", no_argument, NULL, OPT_NO_EFE },
	{ "u8", no_argument, NULL, OPT_UNICODE8 },
	{ "u16", no_argument, NULL, OPT_UNICODE16 },
	{ "utf8", no_argument, NULL, OPT_UTF8 },
	{ "bridge", no_argument, NULL, OPT_BRIDGE },
	{ "closed", no_argument, NULL, OPT_CLOSED },
	{ 0, 0, NULL, 0 },
};

void usage(void)
{
	fprintf(stderr, "mkudffs from " PACKAGE_NAME " " PACKAGE_VERSION "\n"
		"Usage:\n"
		"\tmkudffs [options] device [blocks-count]\n"
		"Options:\n"
		"\t--help, -h\n"
		"\t--label=, -l\n"
		"\t--uuid=, -u\n"
		"\t--blocksize=, -b\n"
		"\t--udfrev=, -r\n"
		"\t--lvid=\n"
		"\t--vid=\n"
		"\t--vsid=\n"
		"\t--fsid=\n"
		"\t--fullvsid=\n"
		"\t--uid=\n"
		"\t--gid=\n"
		"\t--bootarea=\n"
		"\t--strategy=\n"
		"\t--spartable=\n"
		"\t--packetlen=\n"
		"\t--media-type=\n"
		"\t--space=\n"
		"\t--ad=\n"
		"\t--noefe\n"
		"\t--u8\n"
		"\t--u16\n"
		"\t--utf8\n"
		"\t--bridge\n"
		"\t--closed\n"
	);
	exit(1);
}

void parse_args(int argc, char *argv[], struct udf_disc *disc, char *device, int *blocksize, int *media_ptr)
{
	int retval;
	int i;
	int media = MEDIA_TYPE_HD;
	uint16_t packetlen = 0;
	unsigned long int blocks = 0;
	char *endptr = NULL;

	while ((retval = getopt_long(argc, argv, "l:u:b:r:h", long_options, NULL)) != EOF)
	{
		switch (retval)
		{
			case OPT_HELP:
			case 'h':
				usage();
				break;
			case OPT_BLK_SIZE:
			case 'b':
			{
				uint16_t bs;

				disc->blocksize = strtoul(optarg, NULL, 0);
				for (bs=512,disc->blocksize_bits=9;
					disc->blocksize_bits<13;
					disc->blocksize_bits++,bs<<=1)
				{
					if (disc->blocksize == bs)
						break;
				}
				if (disc->blocksize_bits == 13)
				{
					fprintf(stderr, "mkudffs: invalid blocksize\n");
					exit(1);
				}
				disc->udf_lvd[0]->logicalBlockSize = cpu_to_le32(disc->blocksize);
				*blocksize = disc->blocksize;
				break;
			}
			case OPT_UDF_REV:
			case 'r':
			{
				if (udf_set_version(disc, strtoul(optarg, NULL, 16)))
				{
					fprintf(stderr, "mkudffs: invalid udf revision\n");
					exit(1);
				}
				break;
			}
			case OPT_NO_EFE:
			{
				disc->flags &= ~FLAG_EFE;
				break;
			}
			case OPT_UNICODE8:
			{
				disc->flags &= ~FLAG_CHARSET;
				disc->flags |= FLAG_UNICODE8;
				if (strcmp(argv[1], "--u8") != 0)
				{
					fprintf(stderr, "mkudffs: Option --u8 must be specified as first argument\n");
					exit(1);
				}
				break;
			}
			case OPT_UNICODE16:
			{
				disc->flags &= ~FLAG_CHARSET;
				disc->flags |= FLAG_UNICODE16;
				if (strcmp(argv[1], "--u16") != 0)
				{
					fprintf(stderr, "mkudffs: Option --u16 must be specified as first argument\n");
					exit(1);
				}
				break;
			}
			case OPT_UTF8:
			{
				disc->flags &= ~FLAG_CHARSET;
				disc->flags |= FLAG_UTF8;
				if (strcmp(argv[1], "--utf8") != 0)
				{
					fprintf(stderr, "mkudffs: Option --utf8 must be specified as first argument\n");
					exit(1);
				}
				break;
			}
			case OPT_BRIDGE:
			{
				disc->flags |= FLAG_BRIDGE;
				break;
			}
			case OPT_CLOSED:
			{
				disc->flags |= FLAG_CLOSED;
				break;
			}
			case OPT_LVID:
			case OPT_VID:
			case OPT_LABEL:
			case 'l':
			{
				if (retval != OPT_VID)
				{
					if (encode_string(disc, disc->udf_lvd[0]->logicalVolIdent, optarg, 128) == (size_t)-1)
					{
						fprintf(stderr, "mkudffs: Error: lvid option is too long\n");
						exit(1);
					}
					memcpy(((struct impUseVolDescImpUse *)disc->udf_iuvd[0]->impUse)->logicalVolIdent, disc->udf_lvd[0]->logicalVolIdent, 128);
					memcpy(disc->udf_fsd->logicalVolIdent, disc->udf_lvd[0]->logicalVolIdent, 128);
				}
				if (retval != OPT_LVID)
				{
					if (encode_string(disc, disc->udf_pvd[0]->volIdent, optarg, 32) == (size_t)-1)
					{
						fprintf(stderr, "mkudffs: Error: vid option is too long\n");
						exit(1);
					}
				}
				break;
			}
			case OPT_VSID:
			{
				dstring ts[128];
				size_t len = encode_string(disc, ts, optarg, 128);
				if (len == (size_t)-1 || (ts[0] == 16 && len > 127-16*2) || (disc->udf_pvd[0]->volSetIdent[0] == 16 && len > (size_t)(127-16*(ts[0]/8))) || len > 127-16)
				{
					fprintf(stderr, "mkudffs: Error: vsid option is too long\n");
					exit(1);
				}
				if (ts[0] == disc->udf_pvd[0]->volSetIdent[0])
				{
					for (i = 0; i < 16; ++i)
					{
						if (ts[0] == 8)
						{
							if (!disc->udf_pvd[0]->volSetIdent[i+1])
								disc->udf_pvd[0]->volSetIdent[i+1] = '0';
						}
						else
						{
							if (!disc->udf_pvd[0]->volSetIdent[(2*i)+1] && !disc->udf_pvd[0]->volSetIdent[(2*i)+2])
								disc->udf_pvd[0]->volSetIdent[(2*i)+2] = '0';
						}
					}
				}
				else if(ts[0] == 16)
				{
					disc->udf_pvd[0]->volSetIdent[0] = 16;
					for (i = 17; i > 0; --i)
					{
						disc->udf_pvd[0]->volSetIdent[(2*(i-1))+2] = disc->udf_pvd[0]->volSetIdent[i];
						disc->udf_pvd[0]->volSetIdent[(2*(i-1))+1] = 0;
					}
				}
				else if(ts[0] == 8)
				{
					ts[0] = 16;
					for (i = len; i > 0; --i)
					{
						ts[(2*(i-1))+2] = ts[i];
						ts[(2*(i-1))+1] = 0;
					}
					len = (len-1)*2+1;
				}
				memcpy(&disc->udf_pvd[0]->volSetIdent[1+16*(ts[0]/8)], &ts[1], len-1+(ts[0]/8));
				disc->udf_pvd[0]->volSetIdent[127] = 16*(ts[0]/8)+len;
				break;
			}
			case OPT_UUID:
			case 'u':
			{
				if (strlen(optarg) != 16)
				{
					fprintf(stderr, "mkudffs: Error: uuid is not 16 bytes length\n");
					exit(1);
				}
				for (i = 0; i < 16; ++i)
				{
					if (!isxdigit(optarg[i]) || (!isdigit(optarg[i]) && !islower(optarg[i])))
					{
						fprintf(stderr, "mkudffs: Error: uuid is not in lowercase hexadecimal digit format\n");
						exit(1);
					}
				}
				if (disc->udf_pvd[0]->volSetIdent[0] == 8)
				{
					memcpy(&disc->udf_pvd[0]->volSetIdent[1], optarg, 16);
					if (disc->udf_pvd[0]->volSetIdent[127] < 17)
					{
						disc->udf_pvd[0]->volSetIdent[17] = 0;
						disc->udf_pvd[0]->volSetIdent[127] = 17;
					}
				}
				else if (disc->udf_pvd[0]->volSetIdent[0] == 16)
				{
					for (i = 0; i < 16; ++i)
					{
						disc->udf_pvd[0]->volSetIdent[2*i+1] = 0;
						disc->udf_pvd[0]->volSetIdent[2*i+2] = optarg[i];
					}
					if (disc->udf_pvd[0]->volSetIdent[127] < 2*16+1)
					{
						disc->udf_pvd[0]->volSetIdent[2*16+1] = 0;
						disc->udf_pvd[0]->volSetIdent[2*16+2] = 0;
						disc->udf_pvd[0]->volSetIdent[127] = 2*16+1;
					}
				}
				break;
			}
			case OPT_FULLVSID:
			{
				if (encode_string(disc, disc->udf_pvd[0]->volSetIdent, optarg, 128) == (size_t)-1)
				{
					fprintf(stderr, "mkudffs: Error: fullvsid option is too long\n");
					exit(1);
				}
				break;
			}
			case OPT_FSID:
			{
				if (encode_string(disc, disc->udf_fsd->fileSetIdent, optarg, 32) == (size_t)-1)
				{
					fprintf(stderr, "mkudffs: Error: fsid option is too long\n");
					exit(1);
				}
				break;
			}
			case OPT_UID:
			{
				disc->uid = strtol(optarg, NULL, 0);
				break;
			}
			case OPT_GID:
			{
				disc->gid = strtol(optarg, NULL, 0);
				break;
			}
			case OPT_BOOTAREA:
			{
				disc->flags &= ~FLAG_BOOTAREA_MASK;
				if (!strcmp(optarg, "preserve"))
					disc->flags |= FLAG_BOOTAREA_PRESERVE;
				else if (!strcmp(optarg, "erase"))
					disc->flags |= FLAG_BOOTAREA_ERASE;
				else if (!strcmp(optarg, "mbr"))
					disc->flags |= FLAG_BOOTAREA_MBR;
				else
				{
					fprintf(stderr, "mkudffs: invalid bootarea option\n");
					exit(1);
				}
				break;
			}
			case OPT_STRATEGY:
			{
				uint16_t strategy;

				strategy = strtoul(optarg, NULL, 0);
				if (strategy == 4096)
					disc->flags |= FLAG_STRATEGY4096;
				else if (strategy != 4)
				{
					fprintf(stderr, "mkudffs: invalid strategy type\n");
					exit(1);
				}
				break;
			}
			case OPT_SPARTABLE:
			{
				uint8_t spartable;

				spartable = strtoul(optarg, NULL, 0);
				if (spartable > 4)
				{
					fprintf(stderr, "mkudffs: invalid spartable count\n");
					exit(1);
				}
				add_type2_sparable_partition(disc, 0, spartable, packetlen);
				media = MEDIA_TYPE_CDRW;
				break;
			}
			case OPT_PACKETLEN:
			{
				struct sparablePartitionMap *spm;

				packetlen = strtoul(optarg, NULL, 0);
				if ((spm = find_type2_sparable_partition(disc, 0)))
					spm->packetLength = cpu_to_le16(packetlen);
				break;
			}
			case OPT_MEDIA_TYPE:
			{
				if (!strcmp(optarg, "hd"))
				{
					disc->udf_pd[0]->accessType = cpu_to_le32(PD_ACCESS_TYPE_OVERWRITABLE);
					media = MEDIA_TYPE_HD;
				}
				else if (!strcmp(optarg, "dvd"))
				{
					disc->udf_pd[0]->accessType = cpu_to_le32(PD_ACCESS_TYPE_READ_ONLY);
					media = MEDIA_TYPE_DVD;
				}
				else if (!strcmp(optarg, "dvdram"))
				{
					disc->udf_pd[0]->accessType = cpu_to_le32(PD_ACCESS_TYPE_OVERWRITABLE);
					media = MEDIA_TYPE_DVDRAM;
				}
				else if (!strcmp(optarg, "dvdrw"))
				{
					disc->udf_pd[0]->accessType = cpu_to_le32(PD_ACCESS_TYPE_OVERWRITABLE);
					media = MEDIA_TYPE_DVDRW;
					packetlen = 16;
				}
				else if (!strcmp(optarg, "worm"))
				{
					disc->udf_pd[0]->accessType = cpu_to_le32(PD_ACCESS_TYPE_WRITE_ONCE);
					media = MEDIA_TYPE_WORM;
					disc->flags |= (FLAG_STRATEGY4096 | FLAG_BLANK_TERMINAL);
				}
				else if (!strcmp(optarg, "mo"))
				{
					disc->udf_pd[0]->accessType = cpu_to_le32(PD_ACCESS_TYPE_REWRITABLE);
					media = MEDIA_TYPE_MO;
					disc->flags |= (FLAG_STRATEGY4096 | FLAG_BLANK_TERMINAL);
				}
				else if (!strcmp(optarg, "cdrw"))
				{
					disc->udf_pd[0]->accessType = cpu_to_le32(PD_ACCESS_TYPE_REWRITABLE);
					media = MEDIA_TYPE_CDRW;
				}
				else if (!strcmp(optarg, "cdr"))
				{
					disc->udf_pd[0]->accessType = cpu_to_le32(PD_ACCESS_TYPE_WRITE_ONCE);
					media = MEDIA_TYPE_CDR;
					disc->flags |= FLAG_VAT;
					disc->flags &= ~FLAG_CLOSED;
				}
				else
				{
					fprintf(stderr, "mkudffs: invalid media type\n");
					exit(1);
				}
				break;
			}
			case OPT_SPACE:
			{
				if (!strncmp(optarg, "freedbitmap", 11))
					disc->flags |= FLAG_FREED_BITMAP;
				else if (!strncmp(optarg, "freedtable", 10))
					disc->flags |= FLAG_FREED_TABLE;
				else if (!strncmp(optarg, "unallocbitmap", 13))
					disc->flags |= FLAG_UNALLOC_BITMAP;
				else if (!strncmp(optarg, "unalloctable", 12))
					disc->flags |= FLAG_UNALLOC_TABLE;
				else
				{
					fprintf(stderr, "mkudffs: invalid space type\n");
					exit(1);
				}
				break;
			}
			case OPT_AD:
			{
				if (!strncmp(optarg, "inicb", 5))
				{
					default_fe.icbTag.flags = cpu_to_le16(ICBTAG_FLAG_AD_IN_ICB);
					default_efe.icbTag.flags = cpu_to_le16(ICBTAG_FLAG_AD_IN_ICB);
				}
				else if (!strncmp(optarg, "short", 5))
				{
					default_fe.icbTag.flags = cpu_to_le16(ICBTAG_FLAG_AD_SHORT);
					default_efe.icbTag.flags = cpu_to_le16(ICBTAG_FLAG_AD_SHORT);
				}
				else if (!strncmp(optarg, "long", 4))
				{
					default_fe.icbTag.flags = cpu_to_le16(ICBTAG_FLAG_AD_LONG);
					default_efe.icbTag.flags = cpu_to_le16(ICBTAG_FLAG_AD_LONG);
				}
				else
				{
					fprintf(stderr, "mkudffs: invalid allocation descriptor\n");
					exit(1);
				}
				break;
			}
			default:
				exit(1);
		}
	}
	if (optind == argc)
		usage();
	strncpy(device, argv[optind], NAME_MAX-1);
	device[NAME_MAX-1] = '\0';
	optind ++;
	if (optind < argc)
	{
		errno = 0;
		blocks = strtoul(argv[optind++], &endptr, 0);
		if (errno || *endptr || blocks > UINT32_MAX)
		{
			fprintf(stderr, "mkudffs: invalid block-count\n");
			exit(1);
		}
		disc->blocks = blocks;
	}
	if (optind < argc)
		usage();

	if (le32_to_cpu(disc->udf_lvd[0]->numPartitionMaps) == 0)
	{
		if ((media == MEDIA_TYPE_CDRW || media == MEDIA_TYPE_DVDRW) && (disc->udf_rev != 0x0102))
			add_type2_sparable_partition(disc, 0, 2, packetlen);
		else if ((media == MEDIA_TYPE_CDR) && (disc->udf_rev != 0x0102))
		{
			add_type1_partition(disc, 0);
			add_type2_virtual_partition(disc, 0);
		}
		else
			add_type1_partition(disc, 0);
	}

	if (!(disc->flags & FLAG_SPACE))
		disc->flags |= FLAG_UNALLOC_BITMAP;

	if (media == MEDIA_TYPE_CDR)
		disc->flags &= ~FLAG_SPACE;

	if (!(disc->flags & FLAG_BOOTAREA_MASK))
	{
		if (media == MEDIA_TYPE_HD)
			disc->flags |= FLAG_BOOTAREA_ERASE;
		else
			disc->flags |= FLAG_BOOTAREA_PRESERVE;
	}

	for (i=0; i<UDF_ALLOC_TYPE_SIZE; i++)
	{
		if (disc->sizing[i].denomSize == 0)
			disc->sizing[i] = default_sizing[default_media[media]][i];
	}

	*media_ptr = media;
}
