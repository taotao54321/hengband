﻿#include "core/show-file.h"
#include "io/files.h"
#include "core/angband-version.h"
#include "term/gameterm.h"

/*!
 * todo 表示とそれ以外を分割する
 * @brief ファイル内容の一行をコンソールに出力する
 * Display single line of on-line help file
 * @param str 出力する文字列
 * @param cy コンソールの行
 * @param shower 確認中
 * @return なし
 * @details
 * <pre>
 * You can insert some special color tag to change text color.
 * Such as...
 * WHITETEXT [[[[y|SOME TEXT WHICH IS DISPLAYED IN YELLOW| WHITETEXT
 * A colored segment is between "[[[[y|" and the last "|".
 * You can use any single character in place of the "|".
 * </pre>
 */
static void show_file_aux_line(concptr str, int cy, concptr shower)
{
	char lcstr[1024];
	if (shower)
	{
		strcpy(lcstr, str);
		str_tolower(lcstr);
	}

	int cx = 0;
	Term_gotoxy(cx, cy);

	static const char tag_str[] = "[[[[";
	byte color = TERM_WHITE;
	char in_tag = '\0';
	for (int i = 0; str[i];)
	{
		int len = strlen(&str[i]);
		int showercol = len + 1;
		int bracketcol = len + 1;
		int endcol = len;
		concptr ptr;
		if (shower)
		{
			ptr = my_strstr(&lcstr[i], shower);
			if (ptr) showercol = ptr - &lcstr[i];
		}

		ptr = in_tag ? my_strchr(&str[i], in_tag) : my_strstr(&str[i], tag_str);
		if (ptr) bracketcol = ptr - &str[i];
		if (bracketcol < endcol) endcol = bracketcol;
		if (showercol < endcol) endcol = showercol;

		Term_addstr(endcol, color, &str[i]);
		cx += endcol;
		i += endcol;

		if (shower && endcol == showercol)
		{
			int showerlen = strlen(shower);
			Term_addstr(showerlen, TERM_YELLOW, &str[i]);
			cx += showerlen;
			i += showerlen;
			continue;
		}

		if (endcol != bracketcol) continue;

		if (in_tag)
		{
			i++;
			in_tag = '\0';
			color = TERM_WHITE;
			continue;
		}

		i += sizeof(tag_str) - 1;
		color = color_char_to_attr(str[i]);
		if (color == 255 || str[i + 1] == '\0')
		{
			color = TERM_WHITE;
			Term_addstr(-1, TERM_WHITE, tag_str);
			cx += sizeof(tag_str) - 1;
			continue;
		}

		i++;
		in_tag = str[i];
		i++;
	}

	Term_erase(cx, cy, 255);
}


/*!
 * todo 表示とそれ以外を分割する
 * @brief ファイル内容をコンソールに出力する
 * Recursive file perusal.
 * @param creature_ptr プレーヤーへの参照ポインタ
 * @param show_version TRUEならばコンソール上にゲームのバージョンを表示する
 * @param name ファイル名の文字列
 * @param what 内容キャプションの文字列
 * @param line 表示の現在行
 * @param mode オプション
 * @return なし
 * @details
 * <pre>
 * Process various special text in the input file, including
 * the "menu" structures used by the "help file" system.
 * Return FALSE on 'q' to exit from a deep, otherwise TRUE.
 * </pre>
 */
bool show_file(player_type *creature_ptr, bool show_version, concptr name, concptr what, int line, BIT_FLAGS mode)
{
	int wid, hgt;
	Term_get_size(&wid, &hgt);

	char finder_str[81];
	strcpy(finder_str, "");

	char shower_str[81];
	strcpy(shower_str, "");

	char caption[128];
	strcpy(caption, "");

	char hook[68][32];
	for (int i = 0; i < 68; i++)
	{
		hook[i][0] = '\0';
	}

	char filename[1024];
	strcpy(filename, name);
	int n = strlen(filename);

	concptr tag = NULL;
	for (int i = 0; i < n; i++)
	{
		if (filename[i] == '#')
		{
			filename[i] = '\0';
			tag = filename + i + 1;
			break;
		}
	}

	name = filename;
	FILE *fff = NULL;
	char path[1024];
	if (what)
	{
		strcpy(caption, what);
		strcpy(path, name);
		fff = my_fopen(path, "r");
	}

	if (!fff)
	{
		sprintf(caption, _("ヘルプ・ファイル'%s'", "Help file '%s'"), name);
		path_build(path, sizeof(path), ANGBAND_DIR_HELP, name);
		fff = my_fopen(path, "r");
	}

	if (!fff)
	{
		sprintf(caption, _("スポイラー・ファイル'%s'", "Info file '%s'"), name);
		path_build(path, sizeof(path), ANGBAND_DIR_INFO, name);
		fff = my_fopen(path, "r");
	}

	if (!fff)
	{
		path_build(path, sizeof(path), ANGBAND_DIR, name);

		for (int i = 0; path[i]; i++)
			if ('\\' == path[i])
				path[i] = PATH_SEP[0];

		sprintf(caption, _("スポイラー・ファイル'%s'", "Info file '%s'"), name);
		fff = my_fopen(path, "r");
	}

	if (!fff)
	{
		msg_format(_("'%s'をオープンできません。", "Cannot open '%s'."), name);
		msg_print(NULL);

		return TRUE;
	}

	int skey;
	int next = 0;
	int size = 0;
	int back = 0;
	bool menu = FALSE;
	char buf[1024];
	bool reverse = (line < 0);
	while (TRUE)
	{
		char *str = buf;
		if (my_fgets(fff, buf, sizeof(buf))) break;
		if (!prefix(str, "***** "))
		{
			next++;
			continue;
		}

		if ((str[6] == '[') && isalpha(str[7]))
		{
			int k = str[7] - 'A';
			menu = TRUE;
			if ((str[8] == ']') && (str[9] == ' '))
			{
				strncpy(hook[k], str + 10, 31);
				hook[k][31] = '\0';
			}

			continue;
		}

		if (str[6] != '<') continue;

		size_t len = strlen(str);
		if (str[len - 1] == '>')
		{
			str[len - 1] = '\0';
			if (tag && streq(str + 7, tag)) line = next;
		}
	}

	size = next;
	int rows = hgt - 4;
	if (line == -1)
		line = ((size - 1) / rows)*rows;

	Term_clear();

	concptr find = NULL;
	while (TRUE)
	{
		if (line >= size - rows)
			line = size - rows;
		if (line < 0) line = 0;

		if (next > line)
		{
			my_fclose(fff);
			fff = my_fopen(path, "r");
			if (!fff) return FALSE;

			next = 0;
		}

		while (next < line)
		{
			if (my_fgets(fff, buf, sizeof(buf))) break;
			if (prefix(buf, "***** ")) continue;
			next++;
		}

		int row_count = 0;
		concptr shower = NULL;
		for (int i = 0; i < rows; i++)
		{
			concptr str = buf;
			if (!i) line = next;
			if (my_fgets(fff, buf, sizeof(buf))) break;
			if (prefix(buf, "***** ")) continue;
			next++;
			if (find && !i)
			{
				char lc_buf[1024];
				strcpy(lc_buf, str);
				str_tolower(lc_buf);
				if (!my_strstr(lc_buf, find)) continue;
			}

			find = NULL;
			show_file_aux_line(str, i + 2, shower);
			row_count++;
		}

		while (row_count < rows)
		{
			Term_erase(0, row_count + 2, 255);
			row_count++;
		}

		if (find)
		{
			bell();
			line = back;
			find = NULL;
			continue;
		}

		if (show_version)
		{
			prt(format(_("[変愚蛮怒 %d.%d.%d, %s, %d/%d]", "[Hengband %d.%d.%d, %s, Line %d/%d]"),
				FAKE_VER_MAJOR - 10, FAKE_VER_MINOR, FAKE_VER_PATCH,
				caption, line, size), 0, 0);
		}
		else
		{
			prt(format(_("[%s, %d/%d]", "[%s, Line %d/%d]"),
				caption, line, size), 0, 0);
		}

		if (size <= rows)
		{
			prt(_("[キー:(?)ヘルプ (ESC)終了]", "[Press ESC to exit.]"), hgt - 1, 0);
		}
		else
		{
#ifdef JP
			if (reverse)
				prt("[キー:(RET/スペース)↑ (-)↓ (?)ヘルプ (ESC)終了]", hgt - 1, 0);
			else
				prt("[キー:(RET/スペース)↓ (-)↑ (?)ヘルプ (ESC)終了]", hgt - 1, 0);
#else
			prt("[Press Return, Space, -, =, /, |, or ESC to exit.]", hgt - 1, 0);
#endif
		}

		skey = inkey_special(TRUE);
		switch (skey)
		{
		case '?':
			if (strcmp(name, _("jhelpinfo.txt", "helpinfo.txt")) != 0)
				show_file(creature_ptr, TRUE, _("jhelpinfo.txt", "helpinfo.txt"), NULL, 0, mode);
			break;
		case '=':
			prt(_("強調: ", "Show: "), hgt - 1, 0);

			char back_str[81];
			strcpy(back_str, shower_str);
			if (askfor(shower_str, 80))
			{
				if (shower_str[0])
				{
					str_tolower(shower_str);
					shower = shower_str;
				}
				else shower = NULL;
			}
			else strcpy(shower_str, back_str);
			break;

		case '/':
		case KTRL('s'):
			prt(_("検索: ", "Find: "), hgt - 1, 0);
			strcpy(back_str, finder_str);
			if (askfor(finder_str, 80))
			{
				if (finder_str[0])
				{
					find = finder_str;
					back = line;
					line = line + 1;
					str_tolower(finder_str);
					shower = finder_str;
				}
				else shower = NULL;
			}
			else strcpy(finder_str, back_str);
			break;

		case '#':
		{
			char tmp[81];
			prt(_("行: ", "Goto Line: "), hgt - 1, 0);
			strcpy(tmp, "0");

			if (askfor(tmp, 80)) line = atoi(tmp);
			break;
		}

		case SKEY_TOP:
			line = 0;
			break;

		case SKEY_BOTTOM:
			line = ((size - 1) / rows) * rows;
			break;

		case '%':
		{
			char tmp[81];
			prt(_("ファイル・ネーム: ", "Goto File: "), hgt - 1, 0);
			strcpy(tmp, _("jhelp.hlp", "help.hlp"));

			if (askfor(tmp, 80))
			{
				if (!show_file(creature_ptr, TRUE, tmp, NULL, 0, mode)) skey = 'q';
			}

			break;
		}

		case '-':
			line = line + (reverse ? rows : -rows);
			if (line < 0) line = 0;
			break;

		case SKEY_PGUP:
			line = line - rows;
			if (line < 0) line = 0;
			break;

		case '\n':
		case '\r':
			line = line + (reverse ? -1 : 1);
			if (line < 0) line = 0;
			break;

		case '8':
		case SKEY_UP:
			line--;
			if (line < 0) line = 0;
			break;

		case '2':
		case SKEY_DOWN:
			line++;
			break;

		case ' ':
			line = line + (reverse ? -rows : rows);
			if (line < 0) line = 0;
			break;

		case SKEY_PGDOWN:
			line = line + rows;
			break;
		}

		if (menu)
		{
			int key = -1;
			if (!(skey & SKEY_MASK) && isalpha(skey))
				key = skey - 'A';

			if ((key > -1) && hook[key][0])
			{
				/* Recurse on that file */
				if (!show_file(creature_ptr, TRUE, hook[key], NULL, 0, mode))
					skey = 'q';
			}
		}

		if (skey == '|')
		{
			FILE *ffp;
			char buff[1024];
			char xtmp[82];

			strcpy(xtmp, "");

			if (!get_string(_("ファイル名: ", "File name: "), xtmp, 80)) continue;
			my_fclose(fff);
			path_build(buff, sizeof(buff), ANGBAND_DIR_USER, xtmp);

			/* Hack -- Re-Open the file */
			fff = my_fopen(path, "r");

			ffp = my_fopen(buff, "w");

			if (!(fff && ffp))
			{
				msg_print(_("ファイルを開けません。", "Failed to open file."));
				skey = ESCAPE;
				break;
			}

			sprintf(xtmp, "%s: %s", creature_ptr->name, what ? what : caption);
			my_fputs(ffp, xtmp, 80);
			my_fputs(ffp, "\n", 80);

			while (!my_fgets(fff, buff, sizeof(buff)))
				my_fputs(ffp, buff, 80);
			my_fclose(fff);
			my_fclose(ffp);
			fff = my_fopen(path, "r");
		}

		if ((skey == ESCAPE) || (skey == '<')) break;

		if (skey == KTRL('q')) skey = 'q';

		if (skey == 'q') break;
	}

	my_fclose(fff);
	return (skey != 'q');
}
