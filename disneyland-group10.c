#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//***************************** View Data *****************************

/* Configuration */
#define MAX_ROWS 100        // Maximum number of csv rows
#define COLS 6              // Fixed number of columns
#define MAX_CELL 2048       // Maximum length of one cell
#define MAX_REVIEW_WIDTH 52 // Maximum width of review text column

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
        "Reviewer_Location", "Review_Text", "Branch"};

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
        "Reviewer_Location", "Review_Text", "Branch"};

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

/* Function to help sort review entries */
void swap_rows(int a, int b)
{
    char temp[MAX_CELL];

    for (int c = 0; c < COLS; c++)
    {
        strcpy(temp, table[a][c]);
        strcpy(table[a][c], table[b][c]);
        strcpy(table[b][c], temp);
    }
}

/* Sorts review entries by rating in a descending order*/
void sort_by_rating_desc()
{
    for (int i = 0; i < rows - 1; i++)
    {
        for (int j = 0; j < rows - i - 1; j++)
        {
            int r1 = atoi(table[j][1]);
            int r2 = atoi(table[j + 1][1]);

            if (r1 < r2)
            {
                char temp[COLS][MAX_CELL];
                memcpy(temp, table[j], sizeof(temp));
                memcpy(table[j], table[j + 1], sizeof(temp));
                memcpy(table[j + 1], temp, sizeof(temp));
            }
        }
    }
}

/* Sorting review entries alphabetically by branch */
void sort_by_branch(void)
{
    for (int i = 0; i < rows - 1; i++)
    {
        for (int j = i + 1; j < rows; j++)
        {
            if (strcmp(table[i][5], table[j][5]) > 0)
            {
                swap_rows(i, j);
            }
        }
    }
}

/* Menu for sorting */
int sort_menu()
{
    // Input is read as char
    char input[10];

    int choice;

    printf("\nSort reviews by:\n\n");
    printf("1 No sorting\n");
    printf("2 Rating (high to low)\n");
    printf("3 Branch (A-Z)\n\n");
    printf("Choice: ");

    if (!fgets(input, sizeof(input), stdin))
    {
        return 0;
    }

    if (sscanf(input, "%d", &choice) != 1)
    {
        printf("Please enter a number!\n\n");
        return 0;
    }

    if (choice < 1 || choice > 3)
    {
        printf("Please enter a number between 1 and 3!\n\n");
        return 0;
    }

    if (choice == 2)
    {
        sort_by_rating_desc();
    }
    else if (choice == 3)
    {
        sort_by_branch();
    }

    return 1;
}

// function only 12 month
void inputMonth(char *result, int maxLen)
{
    const char *months[] = {
        "January", "February", "March", "April",
        "May", "June", "July", "August",
        "September", "October", "November", "December"};

    char input[50];
    int valid;

    while (1)
    {
        printf("Enter the month you have visited (e.g. April): ");

        if (!fgets(input, sizeof(input), stdin))
            continue;

        // delete newline
        input[strcspn(input, "\n")] = '\0';

        valid = 0;
        for (int i = 0; i < 12; i++)
        {
            if (strcmp(input, months[i]) == 0)
            {
                valid = 1;
                break;
            }
        }

        if (valid)
        {
            strncpy(result, input, maxLen - 1);
            result[maxLen - 1] = '\0';
            return;
        }

        printf("Invalid month. Please enter a valid month name.\n");
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
    int ch;
    char month[100];
    char location[200];
    char review_text[2000];
    char branch[200];

    while (1)
    {
        printf("Enter your rating (1-5): ");

        if (scanf("%d", &rating) != 1)
        {
            printf("Rating must be a number!\n");
            while ((ch = getchar()) != '\n' && ch != EOF) { }
            continue;
        }

        while ((ch = getchar()) != '\n' && ch != EOF) { }

        if (rating < 1 || rating > 5)
        {
            printf("Rating must be between 1 and 5!\n");
            continue;
        }

        break;
    }

    inputMonth(month, sizeof(month));

    for (i = 0; month[i] != '\0'; i++)
    {
        if (month[i] >= '0' && month[i] <= '9')
        {
            printf("Month must not contain numbers!\n");
            return;
        }
    }

    while (1)
    {
        printf("Enter your location: ");
        if (scanf(" %199[^\n]", location) != 1)
        {
            printf("Location input is invalid!\n");
            while ((ch = getchar()) != '\n' && ch != EOF) { }
            continue;
        }
        getchar();

        for (i = 0; location[i] != '\0'; i++)
        {
            if (location[i] >= '0' && location[i] <= '9')
            {
                printf("Location must not contain numbers!\n");
                break;
            }
        }

        if (location[i] == '\0')
            break;
    }

    printf("Enter your review: ");
    scanf(" %1999[^\n]", review_text);
    getchar();

    while (1)
    {
        printf("Enter the branch you have visited (e.g. Disneyland_Paris): ");
        if (scanf(" %199[^\n]", branch) != 1)
        {
            printf("Branch input is invalid!\n");
            while ((ch = getchar()) != '\n' && ch != EOF) { }
            continue;
        }
        getchar();

        for (i = 0; branch[i] != '\0'; i++)
        {
            if (branch[i] >= '0' && branch[i] <= '9')
            {
                printf("Branch must not contain numbers!\n");
                break;
            }
        }

        if (branch[i] == '\0')
            break;
    }

    printf("\n");

    FILE *dl = fopen(filename, "a");
    if (dl == NULL)
    {
        printf("File not found.\n");
        return;
    }

    if (!existed)
    {
        fprintf(dl, "Review_ID,Rating,Review_Month,Reviewer_Location,Review_Text,Branch\n");
    }

    /* make sure the file ends with newline before appending */
    FILE *check = fopen(filename, "r+");
    if (check != NULL)
    {
        fseek(check, -1, SEEK_END);
        int lastChar = fgetc(check);

        if (lastChar != '\n')
        {
            fseek(check, 0, SEEK_END);
            fputc('\n', check);
        }
        fclose(check);
    }

    fprintf(dl, "%d,%d,", next_id, rating);

    write_csv_field(dl, month);
    fputc(',', dl);
    write_csv_field(dl, location);
    fputc(',', dl);
    write_csv_field(dl, review_text);
    fputc(',', dl);
    write_csv_field(dl, branch);
    fputc('\n', dl);

    fclose(dl);

    printf("\nThank you! We have successfully received your review.\n");
}

//***************************** Delete Data *****************************

/* Trim newline characters from the end of the input string */
static void trim_newline(char *s)
{
    if (!s)
        return;
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r'))
    {
        s[n - 1] = '\0';
        n--;
    }
}

/* Ask the user a yes/no question until valid input is given */
static char ask_yes_no(const char *prompt)
{
    char ans[32];
    while (1)
    {
        printf("%s", prompt);
        if (!fgets(ans, sizeof(ans), stdin))
            return 'n';
        trim_newline(ans);

        if (ans[0] == 'y' || ans[0] == 'Y')
            return 'y';
        if (ans[0] == 'n' || ans[0] == 'N')
            return 'n';

        printf("Please enter y or n.\n");
    }
}

/* NOTE:
 * We intentionally read the CSV file line-by-line (fgets) for deletion.
 * This keeps the delete part simple and avoids an extra "read_csv_record" helper.
 * Assumption: each review is stored in a single CSV line (no embedded newlines).
 */

/* Parse CSV fields correctly */
static int parse_csv_fields(const char *rec, char fields[][2000], int max_fields)
{
    int f = 0, i = 0;

    while (rec[i] != '\0' && f < max_fields)
    {
        int out = 0;
        while (rec[i] == ' ')
            i++;

        /* Field is enclosed in double quotes */
        if (rec[i] == '"')
        {
            i++;
            while (rec[i] != '\0')
            {
                if (rec[i] == '"' && rec[i + 1] == '"')
                {
                    fields[f][out++] = '"';
                    i += 2;
                }
                else if (rec[i] == '"')
                {
                    i++;
                    break;
                }
                else
                {
                    fields[f][out++] = rec[i++];
                }
            }
            fields[f][out] = '\0';
            while (rec[i] && rec[i] != ',')
                i++;
            if (rec[i] == ',')
                i++;
        }
        else
        {
            /* Normal field without quotes */
            while (rec[i] && rec[i] != ',' && rec[i] != '\n')
                fields[f][out++] = rec[i++];
            fields[f][out] = '\0';
            if (rec[i] == ',')
                i++;
        }
        f++;
    }
    return f;
}

/* Delete a review by Review ID */
void delete_review(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        printf("File not found %s\n", filename);
        return;
    }

    /*Read CSV header*/
    char header[2048];
    if (!fgets(header, sizeof(header), fp))
    {
        printf("Error: CSV header missing.\n");
        fclose(fp);
        return;
    }

    /*Read all records into memory for easier deletion*/
    int cap = 128;
    int nrec = 0;
    char **records = (char **)malloc(sizeof(char *) * cap);
    if (!records)
    {
        fclose(fp);
        return;
    }

    char rec[8000];
    while (fgets(rec, sizeof(rec), fp))
    {
        /* remove trailing newline so later rewriting can control \n consistently */
        trim_newline(rec);

        /* skip empty lines */
        if (rec[0] == '\0')
            continue;

        if (nrec >= cap)
        {
            cap *= 2;
            char **tmp = (char **)realloc(records, sizeof(char *) * cap);
            if (!tmp)
                break;
            records = tmp;
        }

        records[nrec] = (char *)malloc(strlen(rec) + 1);
        if (!records[nrec])
            break;
        strcpy(records[nrec], rec);
        nrec++;
    }
    fclose(fp);

    /* Keep asking until user enters a valid and existing Review ID */
    int delete_id = 0;
    int target_index = -1;
    char line[128];

    while (1)
    {
        printf("Enter the Review ID to delete: ");
        if (!fgets(line, sizeof(line), stdin))
            continue;
        trim_newline(line);

        /* Input must be a number only */
        int value;
        char extra;
        if (sscanf(line, "%d %c", &value, &extra) != 1)
        {
            printf("\nInvalid Review ID. Please enter numbers only.\n\n");
            continue;
        }

        delete_id = value;
        target_index = -1;

        /* Search for the matching Review ID */
        for (int i = 0; i < nrec; i++)
        {
            int id;
            if (sscanf(records[i], "%d,", &id) == 1 && id == delete_id)
            {
                target_index = i;
                break;
            }
        }

        if (target_index == -1)
        {
            printf("\nReview ID not found. Please try again.\n\n");
            continue;
        }

        break;
    }

    /* Display the selected review */
    {
        char fields[6][2000];
        parse_csv_fields(records[target_index], fields, 6);

        printf("\n--- Review Found ---\n");
        printf("ID: %s\nRating: %s\nMonth: %s\nLocation: %s\nReview: %s\nBranch: %s\n",
               fields[0], fields[1], fields[2], fields[3], fields[4], fields[5]);
    }

    /* Double confirmation to prevent accidental deletion */
    if (ask_yes_no("\nDo you want to delete this review? (y/n): ") == 'n')
    {
        printf("\nThis review has NOT been deleted.\n");
        for (int i = 0; i < nrec; i++)
            free(records[i]);
        free(records);
        return;
    }

    if (ask_yes_no("\nAre you sure you want to delete this review? (y/n): ") == 'n')
    {
        printf("\nThis review has NOT been deleted.\n");
        for (int i = 0; i < nrec; i++)
            free(records[i]);
        free(records);
        return;
    }

    /* Rewrite the original CSV file */
    fp = fopen(filename, "w");
    if (!fp)
    {
        printf("\nError: cannot write file.\n");
        for (int i = 0; i < nrec; i++)
            free(records[i]);
        free(records);
        return;
    }

    fputs(header, fp);
    if (header[strlen(header) - 1] != '\n')
        fputc('\n', fp);

    for (int i = 0; i < nrec; i++)
    {
        if (i == target_index)
            continue;
        fputs(records[i], fp);
        if (records[i][strlen(records[i]) - 1] != '\n')
            fputc('\n', fp);
    }
    fclose(fp);

    printf("\nReview deleted successfully.\n");

    for (int i = 0; i < nrec; i++)
        free(records[i]);
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

//comma in review
void parseCSVLine(char *line, struct Review *r)
{
    char fields[6][REVIEW_LEN];
    int field = 0, i = 0, j = 0;
    int in_quotes = 0;

    for (int k = 0; k < 6; k++)
        fields[k][0] = '\0';

    while (line[i] != '\0' && field < 6)
    {
        if (line[i] == '"')
        {
            in_quotes = !in_quotes;
        }
        else if (line[i] == ',' && !in_quotes)
        {
            fields[field][j] = '\0';
            field++;
            j = 0;
        }
        else
        {
            fields[field][j++] = line[i];
        }
        i++;
    }
    fields[field][j] = '\0';

    /* copy to struct */
    r->id = atoi(fields[0]);
    r->rating = atoi(fields[1]);
    strcpy(r->month, fields[2]);
    strcpy(r->location, fields[3]);
    strcpy(r->review, fields[4]);
    strcpy(r->branch, fields[5]);
}

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

        printf("\nPlease enter numbers only!\n");
    }
}
// function loadcsv
void loadCSV()
{
    FILE *fp = fopen("disneylandreview.csv", "r");
    char line[REVIEW_LEN];

    if (!fp)
        return;

    count = 0;

    /* skip header */
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp) && count < MAX)
    {
        // ********** เพิ่มบรรทัดนี้ลงไปครับ **********
        // ลบ \n หรือ \r ที่อยู่ท้ายประโยคออก
        line[strcspn(line, "\r\n")] = '\0'; 
        // *****************************************

        parseCSVLine(line, &reviews[count]);
        count++;
    }

    fclose(fp);
}


int inputRating(const char *message)
{
    char line[100];
    int value;

    while (1)
    {
        printf("%s", message);

        if (!fgets(line, sizeof(line), stdin))
            continue;

        if (sscanf(line, "%d", &value) != 1)
        {
            printf("\nPlease enter numbers only!\n");
            continue;
        }

        if (value < 1 || value > 5)
        {
            printf("\nRating must be between 1 and 5!\n");
            continue;
        }

        return value;
    }
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
    reviews[index].rating = inputRating("Enter the Rating (1-5): ");

    // printf("Enter the month you have visited (e.g. April): ");
    inputMonth(reviews[index].month, sizeof(reviews[index].month));

    printf("Enter your location: ");
    scanf(" %49[^\n]", reviews[index].location);

    printf("Enter your review: ");
    scanf(" %3999[^\n]", reviews[index].review);

    printf("Enter the brranch you have visited (e.g. Disneyland_Paris): ");
    scanf(" %49[^\n]", reviews[index].branch);

    // save file
    saveCSV();

    printf("\nReview updated successfully!\n");
}

// funtion show all process
void editMenu()
{
    count = 0;
    loadCSV(); // use function loadcsv

    // Giving user to Enter review id
    //  use function inputint
    int id; 
    int index; 

    while (1)
    {
        id = inputInt("Enter the Review ID: ");
        index = findByID(id);

        if (index == -1)
        {
            printf("\nReview ID not found! Please try again.\n");
            continue;  
        }

        break;
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

    // clear buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;

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
        while ((c = getchar()) != '\n' && c != EOF)
            ;

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

            while (!sort_menu())
            {
                printf("Try again.\n");
            }

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
