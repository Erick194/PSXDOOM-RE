/* m_password.c -- password encode/decode */

#include "doomdef.h"
#include "p_local.h"
#include "st_main.h"
#include "r_local.h"

void Encode_Password(byte *buff)//L80037BBC()
{
	int i, j, shift, code, pos;
	char tmpbuff[8];
	player_t	*player;
	int maxclip, maxshell, maxcell, maxmisl;
	int val, newval;

	player = &players[consoleplayer];
	D_memset(tmpbuff, 0, 8);

	tmpbuff[0] = ((byte)nextmap & 63) << 2;//map
	tmpbuff[0] |= (byte)gameskill & 3;//skill

	// Weapons
	for (i = 0; i < 7; i++)
	{
		if (player->weaponowned[wp_shotgun + i])
		{
			tmpbuff[1] |= (1 << i);
		}
	}

	//Get Maximun Ammo
	maxclip = maxammo[am_clip];
	maxshell = maxammo[am_shell];
	maxcell = maxammo[am_cell];
	maxmisl = maxammo[am_misl];

	// Backpack
	if (player->backpack)
	{
		maxclip <<= 1;
		maxshell <<= 1;
		maxcell <<= 1;
		maxmisl <<= 1;
		tmpbuff[1] |= 0x80;
	}

	// Clip
	val = (player->ammo[am_clip] << 3) / maxclip;
	if ((player->ammo[am_clip] << 3) % maxclip) { val += 1; }
	tmpbuff[2] = val << 4;

	// Shell
	val = (player->ammo[am_shell] << 3) / maxshell;
	if ((player->ammo[am_shell] << 3) % maxshell) { val += 1; }
	tmpbuff[2] |= val;

	// Cell
	val = (player->ammo[am_cell] << 3) / maxcell;
	if ((player->ammo[am_cell] << 3) % maxcell) { val += 1; }
	tmpbuff[3] = val << 4;

	// Missile
	val = (player->ammo[am_misl] << 3) / maxmisl;
	if ((player->ammo[am_misl] << 3) % maxmisl) { val += 1; }
	tmpbuff[3] |= val;

	// Health
	val = (player->health << 3) / 200;
	if ((player->health << 3) % 200) { val += 1; }
	tmpbuff[4] = val << 4;

	// Armor
	val = (player->armorpoints << 3) / 200;
	if ((player->armorpoints << 3) % 200) { val += 1; }
	tmpbuff[4] |= val;

	// ArmorType
	tmpbuff[5] = (byte)player->armortype << 3;

	//Encode Encrypt System
	for (i = 0; i < 45;)
	{
		newval = 0;
		shift = 16;

		for (j = 4; j >= 0; j--)
		{
			pos = i;
			if (i < 0) { pos = i + 7;}

			pos >>= 3;
			code = tmpbuff[pos] & (0x80 >> (i - (pos << 3)));

			if (code != 0)
				newval |= shift;

			shift >>= 1;
			i++;
		}

		pos = ((i - 1) / 5);
		buff[pos] = newval;
	}

	buff[9] = 0;

	for (i = 0; i < 9; i++)
		buff[9] ^= buff[i];

	for (i = 0; i < 9; i++)
		buff[i] ^= buff[9];
}

int Decode_Password(byte *inbuff, int *levelnum, int *skill, player_t *player)//L80037FB0()
{
	int val, newval, i, j, shift, code, pos;
	byte decbuff[8];
	byte buff[10];

	D_memcpy(buff, inbuff, 10);

	val = 0;

	for (i = 0; i < 9; i++)
		buff[i] ^= buff[9];

	for (i = 0; i < 9; i++)
		val ^= buff[i];

	if (val == buff[9])
	{
		//Decode Encrypt System
		for (i = 0; i < 48;)
		{
			newval = 0;
			shift = 0x80;

			for (j = 7; j >= 0; j--)
			{
				pos = (i / 5);
				code = buff[pos] & (16 >> (i - (pos * 5)));

				if (code != 0)
					newval |= shift;

				shift >>= 1;
				i++;
			}

			pos = (i - 1);
			if (pos < 0) { pos = i + 6; }
			pos >>= 3;

			decbuff[pos] = newval;
		}

		*levelnum = decbuff[0] >> 2;

		if (*levelnum != 0 && (*levelnum < 60))
		{
			*skill = decbuff[0] & 3;

			// ArmorType
			if ((decbuff[5] >> 3) < 3)
			{
				// Health
				if ((decbuff[4] >> 4) != 0)
				{
					if (player != 0)
					{
						// Weapons
						for (i = 0; i < 7; i++)
						{
							if ((decbuff[1] >> i) & 1)
							{
								player->weaponowned[wp_shotgun + i] = 1;
							}
						}

						// Backpack
						if (decbuff[1] & 0x80)
						{
							if (!player->backpack)
							{
								player->backpack = 1;

								for (i = 0; i < 4; i++)
									player->maxammo[i] <<= 1;
							}
						}

						// Clip
						val = decbuff[2] >> 4;
						val *= player->maxammo[am_clip];
						if (val < 0) { val += 7; }
						player->ammo[am_clip] = val >> 3;

						// Shell
						val = decbuff[2] & 0xf;
						val *= player->maxammo[am_shell];
						if (val < 0) { val += 7; }
						player->ammo[am_shell] = val >> 3;

						// Cell
						val = decbuff[3] >> 4;
						val *= player->maxammo[am_cell];
						if (val < 0) { val += 7; }
						player->ammo[am_cell] = val >> 3;

						// Shell
						val = decbuff[3] & 0xf;
						val *= player->maxammo[am_misl];
						if (val < 0) { val += 7; }
						player->ammo[am_misl] = val >> 3;

						// Health
						player->health = (decbuff[4] >> 4) * 25;

						// Armor
						player->armorpoints = (decbuff[4] & 15) * 25;

						// ArmorType
						player->armortype = decbuff[5] >> 3;

						// Apply Health on mobj_t
						player->mo->health = player->health;
					}

					return 1;
				}
			}
		}
	}

	return 0;
}

/*
int Decode_Password_v2(byte *inbuff, int *levelnum, int *skill, player_t *player)//L80037FB0()
{
	int val, newval, i, j, shift, code, pos;
	byte decbuff[8];
	byte buff[10];

	D_memcpy(buff, inbuff, 10);

	val = 0;

	for (i = 0; i < 9; i++)
		buff[i] ^= buff[9];

	for (i = 0; i < 9; i++)
		val ^= buff[i];

	if (val == buff[9])
	{
		//Decode Encrypt System
		for (i = 0; i < 48;)
		{
			newval = 0;
			shift = 0x80;

			for (j = 7; j >= 0; j--)
			{
				pos = (i / 5);
				code = buff[pos] & (16 >> (i - (pos * 5)));

				if (code != 0)
					newval |= shift;

				shift >>= 1;
				i++;
			}

			pos = (i - 1);
			if (pos < 0) { pos = i + 6; }
			pos >>= 3;

			decbuff[pos] = newval;
		}

		*levelnum = decbuff[0] >> 2;

		if (*levelnum != 0 && (*levelnum < 60))
		{
			*skill = decbuff[0] & 3;

			if (((decbuff[2] & 15) < 9) &&	// Shell
				((decbuff[2] >> 4) < 9) &&	// Clip
				((decbuff[3] & 15) < 9) &&	// Missile
				((decbuff[3] >> 4) < 9) &&	// Cell
				((decbuff[4] & 15) < 9) &&	// Armor
				((decbuff[4] >> 4) < 9))	// Health
			{
				// ArmorType
				if ((decbuff[5] >> 3) < 3)
				{
					// Health
					if ((decbuff[4] >> 4) != 0)
					{
						if (player != 0)
						{
							// Weapons
							for (i = 0; i < 7; i++)
							{
								if ((decbuff[1] >> i) & 1)
								{
									player->weaponowned[wp_shotgun + i] = 1;
								}
							}

							// Backpack
							if (decbuff[1] & 0x80)
							{
								if (!player->backpack)
								{
									player->backpack = 1;

									for (i = 0; i < 4; i++)
										player->maxammo[i] <<= 1;
								}
							}

							// Clip
							val = decbuff[2] >> 4;
							val *= player->maxammo[am_clip];
							if (val < 0) { val += 7; }
							player->ammo[am_clip] = val >> 3;

							// Shell
							val = decbuff[2] & 0xf;
							val *= player->maxammo[am_shell];
							if (val < 0) { val += 7; }
							player->ammo[am_shell] = val >> 3;

							// Cell
							val = decbuff[3] >> 4;
							val *= player->maxammo[am_cell];
							if (val < 0) { val += 7; }
							player->ammo[am_cell] = val >> 3;

							// Missile
							val = decbuff[3] & 0xf;
							val *= player->maxammo[am_misl];
							if (val < 0) { val += 7; }
							player->ammo[am_misl] = val >> 3;

							// Health
							player->health = (decbuff[4] >> 4) * 25;

							// Armor
							player->armorpoints = (decbuff[4] & 15) * 25;

							// ArmorType
							player->armortype = decbuff[5] >> 3;

							// Apply Health on mobj_t
							player->mo->health = player->health;
						}

						return 1;
					}
				}
			}
		}
	}

	return 0;
}*/
