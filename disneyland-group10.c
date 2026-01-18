#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//***************************** View Data *****************************

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

//***************************** Add Data *****************************

/*Writes one text field into the CSV. It puts the text in quotes. It doubles any " inside the text. */
void write_csv_field(FILE *dl, char text[])
{
    int i = 0;
    fprintf(dl, "\"");

    while (text[i] != '\0')
    {
        if (text[i] == '"')
            fprintf(dl, "\"");

        fprintf(dl, "%c", text[i]);
        i++;
    }

    fprintf(dl, "\"");
}

/*Checks if the file exists. It tries to open the file in read mode. It returns 1 if it works, otherwise 0.*/
int file_exists(const char *filename)
{
    FILE *dl = fopen(filename, "r");
    if (dl == NULL)
        return 0;

    fclose(dl);
    return 1;
}

/*Finds the next Review_ID. It reads the CSV line by line. It returns the biggest ID + 1.*/
int get_next_id(const char *filename)
{
    FILE *dl = fopen(filename, "r");
    char line[2000];
    int id;
    int last_id = 0;

    if (dl == NULL)
        return 1;

    while (fgets(line, 2000, dl) != NULL)
    {
        if (sscanf(line, "%d,", &id) == 1)
            last_id = id;
    }

    fclose(dl);
    return last_id + 1;
}

/*Asks the user for all review data. It appends the new review at the bottom of the CSV.*/
void add_review_append_only(const char *filename)
{
    int existed = file_exists(filename);
    int next_id = get_next_id(filename);

    int rating;
    int i;
    char month[100];
    char location[200];
    char review_text[2000];
    char branch[200];

    printf("Rating (1-5): ");
    if (scanf("%d", &rating) != 1)
    {
        printf("Error: Rating must be a number.\n");
        return;
    }
    getchar();

    if (rating < 1 || rating > 5)
    {
        printf("Error: Rating must be between 1 and 5.\n");
        return;
    }

    printf("Review_Month (e.g. April): ");
    scanf(" %99[^\n]", month);
    getchar();

    for (i = 0; month[i] != '\0'; i++)
    {
        if (month[i] >= '0' && month[i] <= '9')
        {
            printf("Error: Review_Month must not contain numbers.\n");
            return;
        }
    }

    printf("Reviewer_Location: ");
    scanf(" %199[^\n]", location);
    getchar();

    for (i = 0; location[i] != '\0'; i++)
    {
        if (location[i] >= '0' && location[i] <= '9')
        {
            printf("Error: Reviewer_Location must not contain numbers.\n");
            return;
        }
    }

    printf("Review_Text: ");
    scanf(" %1999[^\n]", review_text);
    getchar();

    printf("Branch (e.g. Disneyland_Paris): ");
    scanf(" %199[^\n]", branch);
    getchar();

    for (i = 0; branch[i] != '\0'; i++)
    {
        if (branch[i] >= '0' && branch[i] <= '9')
        {
            printf("Error: Branch must not contain numbers.\n");
            return;
        }
    }

    printf("\n");

    FILE *dl = fopen(filename, "a");
    if (dl == NULL)
    {
        printf("Error: File not found.\n");
        return;
    }

    if (!existed)
    {
        fprintf(dl, "Review_ID,Rating,Review_Month,Reviewer_Location,Review_Text,Branch\n");
    }

    fprintf(dl, "%d,%d,", next_id, rating);

    write_csv_field(dl, month);       fputc(',', dl);
    write_csv_field(dl, location);    fputc(',', dl);
    write_csv_field(dl, review_text); fputc(',', dl);
    write_csv_field(dl, branch);      fputc('\n', dl);

    fclose(dl);

    printf("Thank you! We have successfully received your review.\n");
}

//***************************** Delete Data *****************************

/*Trim newline characters from input string*/
static void trim_newline(char *s)
{
    if (!s) return;
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r'))
    {
        s[n - 1] = '\0';
        n--;
    }
}

/* Ask user a yes/no question until valid input */
static char ask_yes_no(const char *prompt)
{
    char ans[32];
    while (1)
    {
        printf("%s", prompt);
        if (!fgets(ans, sizeof(ans), stdin)) return 'n';
        trim_newline(ans);

        if (ans[0] == 'y' || ans[0] == 'Y') return 'y';
        if (ans[0] == 'n' || ans[0] == 'N') return 'n';

        printf("Please enter y or n.\n");
    }
}

/* Read ONE CSV record */
static int read_csv_record(FILE *fp, char *out, size_t out_size)
{
    if (!fp || !out || out_size == 0) return 0;

    out[0] = '\0';
    char line[2048];
    int in_quotes = 0;
    size_t used = 0;

    while (fgets(line, sizeof(line), fp))
    {
        size_t len = strlen(line);
        if (used + len + 1 >= out_size) return 0;

        memcpy(out + used, line, len + 1);
        used += len;

        for (size_t i = 0; i < len; i++)
        {
            if (line[i] == '"')
            {
                if (line[i + 1] == '"') i++;      
                else in_quotes = !in_quotes;      
            }
        }

        if (!in_quotes) return 1; 
    }

    return used > 0;
}

/* Parse CSV fields correctly */
static int parse_csv_fields(const char *rec, char fields[][2000], int max_fields)
{
    int f = 0, i = 0;

    while (rec[i] != '\0' && f < max_fields)
    {
        int out = 0;
        while (rec[i] == ' ') i++;

        if (rec[i] == '"')
        {
            i++; // skip opening quote
            while (rec[i] != '\0')
            {
                if (rec[i] == '"' && rec[i + 1] == '"')
                {
                    if (out < 1999) fields[f][out++] = '"';
                    i += 2;
                }
                else if (rec[i] == '"')
                {
                    i++;
                    break;
                }
                else
                {
                    if (out < 1999) fields[f][out++] = rec[i++];
                    else i++;
                }
            }
            fields[f][out] = '\0';

            while (rec[i] && rec[i] != ',') i++;
            if (rec[i] == ',') i++;
        }
        else
        {
            while (rec[i] && rec[i] != ',' && rec[i] != '\n' && rec[i] != '\r')
            {
                if (out < 1999) fields[f][out++] = rec[i++];
                else i++;
            }
            fields[f][out] = '\0';
            if (rec[i] == ',') i++;
        }

        f++;
    }
    return f;
}

/* simple strdup */
static char* my_strdup(const char* s)
{
    if (!s) return NULL;
    size_t n = strlen(s);
    char* p = (char*)malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, s, n + 1);
    return p;
}

void delete_review(const char *filename)
{
    // 1) Read entire CSV into memory 
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        printf("File not found: %s\n", filename);
        return;
    }

    char header[8000];
    if (!read_csv_record(fp, header, sizeof(header)))
    {
        printf("Error: CSV header missing.\n");
        fclose(fp);
        return;
    }

    // Store all records into an array
    int capacity = 256;
    int count = 0;
    char **records = (char**)malloc(sizeof(char*) * capacity);
    if (!records)
    {
        printf("Memory allocation failed.\n");
        fclose(fp);
        return;
    }

    char rec[8000];
    while (read_csv_record(fp, rec, sizeof(rec)))
    {
        if (count >= capacity)
        {
            capacity *= 2;
            char **new_records = (char**)realloc(records, sizeof(char*) * capacity);
            if (!new_records)
            {
                printf("Memory allocation failed.\n");
                // free what we already have
                for (int i = 0; i < count; i++) free(records[i]);
                free(records);
                fclose(fp);
                return;
            }
            records = new_records;
        }
        records[count] = my_strdup(rec);
        if (!records[count])
        {
            printf("Memory allocation failed.\n");
            for (int i = 0; i < count; i++) free(records[i]);
            free(records);
            fclose(fp);
            return;
        }
        count++;
    }
    fclose(fp);

    // 2) Ask Review ID repeatedly 
    while (1)
    {
        char line[64];
        printf("Enter the Review ID to delete (or 'q' to cancel): ");
        if (!fgets(line, sizeof(line), stdin))
        {
            printf("Canceled.\n");
            break;
        }
        trim_newline(line);

        if (line[0] == 'q' || line[0] == 'Q')
        {
            printf("Canceled.\n");
            break;
        }

        int delete_id;
        if (sscanf(line, "%d", &delete_id) != 1)
        {
            printf("Invalid input. Please enter a number.\n");
            continue; 
        }

        // 3) Find record by ID 
        int idx = -1;
        for (int i = 0; i < count; i++)
        {
            int id;
            if (sscanf(records[i], "%d,", &id) == 1 && id == delete_id)
            {
                idx = i;
                break;
            }
        }

        if (idx == -1)
        {
            printf("Review ID not found. Please try again.\n");
            continue; 
        }

        // 4) Show found record fields
        char fields[6][2000];
        int nf = parse_csv_fields(records[idx], fields, 6);

        printf("\n--- Review Found ---\n");
        if (nf >= 6)
        {
            printf("Review_ID: %s\n", fields[0]);
            printf("Rating: %s\n", fields[1]);
            printf("Review_Month: %s\n", fields[2]);
            printf("Reviewer_Location: %s\n", fields[3]);
            printf("Review_Text: %s\n", fields[4]);
            printf("Branch: %s\n", fields[5]);
        }
        else
        {
            printf("(Could not parse all fields. Raw record:)\n%s\n", records[idx]);
        }

        // 5) Confirm twice
        if (ask_yes_no("\nDo you want to delete this review? (y/n): ") == 'n')
        {
            printf("This review has NOT been deleted.\n\n");
            continue;
        }

        if (ask_yes_no("Are you sure you want to delete this review? (y/n): ") == 'n')
        {
            printf("This review has NOT been deleted.\n\n");
            continue;
        }

        // 6) Delete: remove that record from array
        free(records[idx]);
        for (int i = idx; i < count - 1; i++)
        {
            records[i] = records[i + 1];
        }
        count--;

        // 7) Rewrite original CSV 
        FILE *out = fopen(filename, "w");
        if (!out)
        {
            printf("Error: cannot open file for writing.\n");
            break;
        }

        fputs(header, out);
        if (header[strlen(header) - 1] != '\n') fputc('\n', out);

        for (int i = 0; i < count; i++)
        {
            fputs(records[i], out);
            size_t L = strlen(records[i]);
            if (L > 0 && records[i][L - 1] != '\n') fputc('\n', out);
        }

        fclose(out);

        printf("Review deleted successfully.\n\n");
        break; 
    }

    for (int i = 0; i < count; i++) free(records[i]);
    free(records);
}

//***************************** Edit Data *****************************

#define MAX 100 // define max value of review
#define LINE 1024
#define REVIEW_LEN 4000

/* =========================
Struct for collect Review
========================= */
struct Review
{
    int id;                  // Review ID
    int rating;              // Rating
    char month[20];          // Review month
    char location[50];       // Reviewer location
    char review[REVIEW_LEN]; // Review text
    char branch[50];         // Disneyland branch
};

struct Review reviews[MAX]; // array collect all review
int count = 0;              // collect the amount of review

// function check int of id and rating
int inputInt(const char *message)
{
    char line[100];
    int value;

    while (1)
    {
        printf("%s", message);
        // if it not int it will be continue in the while loop
        if (!fgets(line, sizeof(line), stdin))
            continue;
        // condition if it "int" it will return the value
        if (sscanf(line, "%d", &value) == 1)
            return value;

        printf("Please enter numbers only.\n");
    }
}
// function loadcsv
void loadCSV()
{
    FILE *fp = fopen("disneylandreview.csv", "r");

    char line[LINE];
    char buffer[REVIEW_LEN];

    /* skip header */
    fgets(line, LINE, fp);

    /* read data */
    while (fgets(line, LINE, fp) && count < MAX)
    {

        buffer[0] = '\0';

        /* if it still don't have " it have many line because it the review sentence */
        if (strchr(line, '"'))
        {
            strcat(buffer, line);

            /* read until find " again */
            while (!strchr(buffer + 1, '"'))
            {
                if (!fgets(line, LINE, fp))
                    break;
                strcat(buffer, line);
            }
        }
        else
        {
            strcpy(buffer, line);
        }

        sscanf(buffer,
               "%d,%d,%19[^,],%49[^,],\"%3999[^\"]\",%49[^\n]",
               &reviews[count].id,
               &reviews[count].rating,
               reviews[count].month,
               reviews[count].location,
               reviews[count].review,
               reviews[count].branch);

        count++;
    }

    fclose(fp);
}

// save function
void saveCSV()
{
    FILE *fp = fopen("disneylandreview.csv", "w");

    /* write header */
    fprintf(fp,
            "Review_ID,Rating,Review_Month,Reviewer_Location,Review_Text,Branch\n");

    /* write review in file csv */
    for (int i = 0; i < count; i++)
    {
        fprintf(fp, "%d,%d,%s,%s,\"%s\",%s\n",
                reviews[i].id,
                reviews[i].rating,
                reviews[i].month,
                reviews[i].location,
                reviews[i].review,
                reviews[i].branch);
    }

    fclose(fp);
}

// find data by ID
int findByID(int id)
{
    for (int i = 0; i < count; i++)
    {
        if (reviews[i].id == id)
            return i; // find → send index back
    }
    return -1; // can't find
}

// function editReview
void editReview(int index)
{
    printf("\n--- Edit Review ---\n");

    // use function inputint
    reviews[index].rating = inputInt("Enter the Rating: ");

    printf("Enter the Month: ");
    scanf(" %19[^\n]", reviews[index].month);

    printf("Enter the Location: ");
    scanf(" %49[^\n]", reviews[index].location);

    printf("Enter the Review: ");
    scanf(" %3999[^\n]", reviews[index].review);

    printf("Enter the Branch: ");
    scanf(" %49[^\n]", reviews[index].branch);

    // save file
    saveCSV();

    printf("\nReview updated successfully!\n");
}

// funtion show all process
void editMenu()
{
    loadCSV(); // use function loadcsv

    // Giving user to Enter review id
    //  use function inputint
    int id = inputInt("Enter Review ID: ");
    int index = findByID(id);

    if (index == -1)
    {
        printf("Review ID not found\n");
        return;
    }

    /* show data to make sure!! */
    printf("\nID: %d\n", reviews[index].id);
    printf("Rating: %d\n", reviews[index].rating);
    printf("Month: %s\n", reviews[index].month);
    printf("Location: %s\n", reviews[index].location);
    printf("Review: %s\n", reviews[index].review);
    printf("Branch: %s\n", reviews[index].branch);

    char confirm;
    printf("\nDo you want to edit this review? (y/n): ");
    scanf(" %c", &confirm);

    //clear buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    if (confirm == 'y' || confirm == 'Y')
    {
        editReview(index);
    }
    return;
}

//***************************** MENU *****************************

int main(void)
{
    int choice;
    while (1)
    {
        printf("****** Welcome to our Disneyland Reviewing System! ******\n\n");
        printf("Please choose one option to continue by entering a number:\n\n");

        char *options[] = {"1 Display Reviews", "2 Add Review", "3 Delete Review", "4 Edit Review", "5 Exit"};
        int numberOfStrings = sizeof(options) / sizeof(options[0]);

        for (int i = 0; i < numberOfStrings; i++)
        {
            printf("%s", options[i]);

            if (i == numberOfStrings - 1) // If last element of array...
            {
                printf("\n\n"); // ... add two escape sequences
            }
            else
            {
                printf("\n");
            }
        }

        printf("Option: ");
        scanf("%d", &choice);

        // clear buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        switch (choice)
        {
        case 1:
            {
                FILE *fp = fopen("disneylandreview.csv", "r");
                if (!fp)
                {
                    perror("File could not be opened");
                    return 1;
                }

                view_data(fp); // Call View function
                fclose(fp);

                column_width();
                print_table();
                break;
            }
        case 2:
            add_review_append_only("disneylandreview.csv"); // Call Add function
            break;

        case 3:
            delete_review("disneylandreview.csv"); // Call Delete function
            break;

        case 4:
            editMenu(); // Call Edit function
            break;

        case 5:
            printf("Thank you and Goodbye!\n");
            return 0;

        default:
            printf("Invalid option! Please enter a number between 1 and 5.\n");
        }
    }
    return 0;
}
