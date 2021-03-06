/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/* geoiplookup.c
 *
 * Copyright (C) 2006 MaxMind LLC
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <GeoIP.h>
#include <GeoIPCity.h>
#include <GeoIP_internal.h>

#if defined(_WIN32)
# ifndef uint32_t 
typedef unsigned int uint32_t; 
# endif 
#endif 

void geoiplookup(GeoIP* gi,char *hostname,int i);

void usage() {
	fprintf(stderr,"Usage: geoiplookup [-d custom_dir] [-f custom_file] [-v] <ipaddress|hostname>\n");
}

int main (int argc, char *argv[]) {
	char * hostname = NULL;
	char * db_info;
	GeoIP * gi;
	int i;
	char *custom_directory = NULL;
	char *custom_file = NULL;
	int version_flag = 0;

	if (argc < 2) {
		usage();
		exit(1);
	} 
	i = 1;
	while (i < argc) {
		if (strcmp(argv[i],"-v") == 0) {
			version_flag = 1;
		} else if (strcmp(argv[i],"-f") == 0) {
			if ((i+1) < argc){
				i++;
				custom_file = argv[i];
			}
		} else if (strcmp(argv[i],"-d") == 0) {
			if ((i+1) < argc){
				i++;
				custom_directory = argv[i];
			}
		} else {
			hostname = argv[i];
		}
		i++;
	}
	if (hostname == NULL) {
		usage();
		exit(1);
	}

	if (custom_directory != NULL) {
		GeoIP_setup_custom_directory(custom_directory);
	}
	_GeoIP_setup_dbfilename();

	if (custom_file != NULL) {
		gi = GeoIP_open(custom_file, GEOIP_STANDARD);
		
		if (NULL == gi) {
			printf("%s not available, skipping...\n", custom_file);
		} else {
		  i = GeoIP_database_edition(gi);
			if (version_flag == 1) {
				db_info = GeoIP_database_info(gi);
				printf("%s: %s\n",GeoIPDBDescription[i],db_info);
				free(db_info);
			} else {
				geoiplookup(gi,hostname,i);
			}
		}
		GeoIP_delete(gi);
	} else {
		/* iterate through different database types */
		for (i = 0; i < NUM_DB_TYPES; ++i) {
			if (GeoIP_db_avail(i)) {
				gi = GeoIP_open_type(i, GEOIP_STANDARD);
				if (NULL == gi) {
					printf("%s not available, skipping...\n", GeoIPDBDescription[i]);
				} else {
					if (version_flag == 1) {
						db_info = GeoIP_database_info(gi);
						printf("%s: %s\n",GeoIPDBDescription[i],db_info);
						free(db_info);
					} else {
						geoiplookup(gi,hostname,i);
					}
				}
				GeoIP_delete(gi);
			}
		}
	}
	return 0;
}

static const char * _mk_NA( const char * p ){
 return p ? p : "N/A";
}

void
geoiplookup(GeoIP * gi, char *hostname, int i)
{
	const char     *country_code;
	const char     *country_name;
	const char     *domain_name;
	int             netspeed;
	int             country_id;
	GeoIPRegion    *region;
	GeoIPRecord    *gir;
	const char     *org;
	uint32_t        ipnum;
	
	ipnum = _GeoIP_lookupaddress(hostname);
	if (ipnum == 0) {
		printf("%s: can't resolve hostname ( %s )\n", GeoIPDBDescription[i], hostname);

	}
	else {

		if (GEOIP_DOMAIN_EDITION == i) {
			domain_name = GeoIP_name_by_ipnum(gi, ipnum);
			if (domain_name == NULL) {
				printf("%s: IP Address not found\n", GeoIPDBDescription[i]);
			}
			else {
				printf("%s: %s\n", GeoIPDBDescription[i], domain_name);
			}
		}
		else if (GEOIP_COUNTRY_EDITION == i) {
                        country_id = GeoIP_id_by_ipnum(gi, ipnum);
			country_code = GeoIP_country_code[country_id];
			country_name = GeoIP_country_name[country_id];
			if (country_id == 0) {
				printf("%s: IP Address not found\n", GeoIPDBDescription[i]);
			}
			else {
				printf("%s: %s, %s\n", GeoIPDBDescription[i], country_code, country_name);
			}
		}
		else if (GEOIP_REGION_EDITION_REV0 == i || GEOIP_REGION_EDITION_REV1 == i) {
			region = GeoIP_region_by_ipnum(gi, ipnum);
			if (NULL == region) {
				printf("%s: IP Address not found\n", GeoIPDBDescription[i]);
			}
			else {
				printf("%s: %s, %s\n", GeoIPDBDescription[i], region->country_code, region->region);
				GeoIPRegion_delete(region);
			}
		}
		else if (GEOIP_CITY_EDITION_REV0 == i) {
			gir = GeoIP_record_by_ipnum(gi, ipnum);
			if (NULL == gir) {
				printf("%s: IP Address not found\n", GeoIPDBDescription[i]);
			}
			else {
				printf("%s: %s, %s, %s, %s, %f, %f\n", GeoIPDBDescription[i], gir->country_code, _mk_NA(gir->region),
				       _mk_NA(gir->city), _mk_NA(gir->postal_code), gir->latitude, gir->longitude);
			}
		}
		else if (GEOIP_CITY_EDITION_REV1 == i) {
			gir = GeoIP_record_by_ipnum(gi, ipnum);
			if (NULL == gir) {
				printf("%s: IP Address not found\n", GeoIPDBDescription[i]);
			}
			else {
				printf("%s: %s, %s, %s, %s, %f, %f, %d, %d\n", GeoIPDBDescription[i], gir->country_code, _mk_NA(gir->region), _mk_NA(gir->city), _mk_NA(gir->postal_code),
				       gir->latitude, gir->longitude, gir->metro_code, gir->area_code);
			}
		}
		else if (GEOIP_ORG_EDITION == i || GEOIP_ISP_EDITION == i) {
			org = GeoIP_org_by_ipnum(gi, ipnum);
			if (org == NULL) {
				printf("%s: IP Address not found\n", GeoIPDBDescription[i]);
			}
			else {
				printf("%s: %s\n", GeoIPDBDescription[i], org);
			}
		}
		else if (GEOIP_NETSPEED_EDITION == i) {
			netspeed = GeoIP_id_by_ipnum(gi, ipnum);
			if (netspeed == GEOIP_UNKNOWN_SPEED) {
				printf("%s: Unknown\n", GeoIPDBDescription[i]);
			}
			else if (netspeed == GEOIP_DIALUP_SPEED) {
				printf("%s: Dialup\n", GeoIPDBDescription[i]);
			}
			else if (netspeed == GEOIP_CABLEDSL_SPEED) {
				printf("%s: Cable/DSL\n", GeoIPDBDescription[i]);
			}
			else if (netspeed == GEOIP_CORPORATE_SPEED) {
				printf("%s: Corporate\n", GeoIPDBDescription[i]);
			}
		}
		else {

		/*
		 * Silent ignore IPv6 databases. Otherwise we get annoying 
		 * messages whenever we have a mixed environment IPv4 and
		 *  IPv6
		 */
		
		/*
		 * printf("Can not handle database type -- try geoiplookup6\n");
		 */
		;
		}
	}
}
