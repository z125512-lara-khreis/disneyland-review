#include <stdio.h>
#include <string.h>

/* Configuration */
#define MAX_ROWS 100            // Maximum number of csv rows
#define COLS 6                  // Fixed number of columns
#define MAX_CELL 2048           // Maximum length of one cell
#define MAX_REVIEW_WIDTH 52     // Maximum width of review text column

char table[MAX_ROWS][COLS][MAX_CELL]; // Stores csv file in memory
int rows = 0;
int col_width[COLS];

/* Function Prototypes */
void view_data(FILE *fp);
void column_width(void);
void print_table(void);
void print_separator(void);
void print_cell_wrapped(const char *text, int width, int line);

/* Word-wrapping for a single cell */
void print_cell_wrapped(const char *text, int width, int line)
{
    int len = strlen(text);
    int pos = 0;

    // Find the starting position for the requested line
    for (int l = 0; l < line && pos < len; l++)
    {
        int next = pos + width;
        if (next > len)
        {
            next = len;
        }

        int cut = next;
        while (cut > pos && text[cut] != ' ')
        {
            cut--;
        }

        // Force cut when no space is found
        if (cut == pos)
        {
            cut = next;
        }
        pos = cut + 1;
    }

    // Print empty cell when no more text is left
    if (pos >= len)
    {
        printf("%-*s", width, "");
        return;
    }

    // Determine end of the current wrapped line
    int end = pos + width;
    if (end > len)
    {
        end = len;
    }

    int cut = end;
    while (cut > pos && text[cut] != ' ')
    {
        cut--;
    }

    if (cut == pos)
    {
        cut = end;
    }

    printf("%-*.*s", width, cut - pos, text + pos);
}

/* Printing a separator line for displaying table*/
void print_separator(void)
{
    printf("+");

    for (int c = 0; c < COLS; c++)
    {
        for (int i = 0; i < col_width[c] + 2; i++)
        {
            printf("-");
        }
        printf("+");
    }
    printf("\n");
}

/* Calculates ideal width for each column */
void column_width(void)
{
    const char *header[COLS] = {
        "Review_ID", "Rating", "Review_Month",
        "Reviewer_Location", "Review_Text", "Branch"
    };

    for (int c = 0; c < COLS; c++)
    {
        col_width[c] = strlen(header[c]); // Column must be as wide as header (minimum)

        // Find longest cell in current column
        for (int r = 0; r < rows; r++)
        {
            int len = strlen(table[r][c]);
            if (len > col_width[c])
            {
                col_width[c] = len;
            }
        }
    }

    // Limit width of review text column to create optimal table
    if (col_width[4] > MAX_REVIEW_WIDTH)
    {
        col_width[4] = MAX_REVIEW_WIDTH;
    }
}

/* Reads csv file into table array & supports quoted text fields */
void view_data(FILE *fp)
{
    char line[32768];
    rows = 0;

    // Skip header
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp) && rows < MAX_ROWS)
    {
        int col = 0, pos = 0;
        int in_quotes = 0;

        for (int i = 0; line[i] && col < COLS; i++)
        {
            char ch = line[i];

            // Toggle quoted-field state
            if (ch == '"')
            {
                in_quotes = !in_quotes;
                continue;
            }

            // Ignore \r (Windows line ending)
            if (ch == '\r')
            {
                continue;
            }

            // Add separator outside quotes
            if (ch == ',' && !in_quotes)
            {
                table[rows][col][pos] = '\0';
                col++;
                pos = 0;
            }
            else if (ch != '\n' && pos < MAX_CELL - 1)
            {
                table[rows][col][pos++] = ch;
            }
        }

        table[rows][col][pos] = '\0';
        rows++;
    }
}

/* Prints formatted table */
void print_table()
{
    const char *header[COLS] = {
        "Review_ID", "Rating", "Review_Month",
        "Reviewer_Location", "Review_Text", "Branch"
    };

    print_separator();

    // Print header
    printf("|");
    for (int c = 0; c < COLS; c++)
        printf(" %-*s |", col_width[c], header[c]);
    printf("\n");

    print_separator();

    // Print data
    for (int r = 0; r < rows; r++)
    {
        int max_lines = 1;

        // Only columns with wrapping are taken into account
        for (int c = 2; c < COLS; c++)
        {
            int w = col_width[c];
            int lines = (strlen(table[r][c]) + w - 1) / w;
            if (lines > max_lines)
            {
                max_lines = lines;
            }
        }

        // Print wrapped line
        for (int l = 0; l < max_lines; l++)
        {
            printf("|");

            for (int c = 0; c < COLS; c++)
            {
                printf(" ");

                // ID and Rating are NOT wrapped
                if (c < 2)
                {
                    if (l == 0)
                    {
                        printf("%-*s", col_width[c], table[r][c]);
                    }
                    else
                    {
                        printf("%-*s", col_width[c], "");
                    }
                }
                else
                {
                    print_cell_wrapped(table[r][c], col_width[c], l);
                }

                printf(" |");
            }
            printf("\n");
        }
        print_separator();
    }
}

int main(void)
{
    FILE *fp = fopen("disneylandreview.csv", "r");
    if (!fp)
    {
        perror("File could not be opened");
        return 1;
    }

    view_data(fp);
    fclose(fp);

    column_width();
    print_table();

    return 0;
}