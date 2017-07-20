#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sqlite3.h"


struct configuration {
    char* db_name;
    char* devise;
} config_default = {
    "labanque.db",
    "euros"

};

typedef struct configuration Config;

void adminMenu();
void admin(char *pseudo);
void loginAdmin();
void menu();

char* convertToLower(char *newString) {
    int i;
    for(i = 0; i < strlen(newString); i++) {
        newString[i] = tolower(newString[i]);
    }
    return newString;
}


int displaySumAmount(Config getConfig, sqlite3 *db) {
    //char *err_msg = 0;
    sqlite3_stmt *res;
    int i = 0;

    int rc = sqlite3_open(getConfig.db_name, &db);

    if (rc != SQLITE_OK) {

        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);

        return 1;
    }

    char *sql = "SELECT SUM(solde) FROM compte";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

    if (rc != SQLITE_OK) {

        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }
    int step = sqlite3_step(res);

    if (step == SQLITE_ROW) {
        for(i = 0; i< sqlite3_column_count(res); i++) {
             printf("The total sum of all accounts is : %s %s \n", sqlite3_column_text(res, i), getConfig.devise);
        }

    }
    sqlite3_finalize(res);
    sqlite3_close(db);
    return 0;
}

int displayAccountByClientName(Config getConfig, sqlite3 *db, char *prenom, char *nom) {
    //char *err_msg = 0;
    sqlite3_stmt *res;
    int i = 1;
    int choice;

    int rc = sqlite3_open(getConfig.db_name, &db);

    if (rc != SQLITE_OK) {

        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);

        return 1;
    }

    char *sql = "SELECT compte.*, client.prenom, client.nom FROM client, compte WHERE client.id = compte.id_client AND UPPER(client.prenom) = UPPER(@prenom) AND UPPER(client.nom) = UPPER(@nom)";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

    if (rc == SQLITE_OK) {
        int id_default = sqlite3_bind_parameter_index(res, "@prenom");
        sqlite3_bind_text(res, id_default, prenom, -1, SQLITE_TRANSIENT);
        int id_default_2 = sqlite3_bind_parameter_index(res, "@nom");
        sqlite3_bind_text(res, id_default_2, nom, -1, SQLITE_TRANSIENT);
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }

    printf("\n\nFirstname : %s , Lastname : %s \n\n", prenom, nom);
    printf("# | ID | SOLDE | TAUX \n\n");
    while ( (rc = sqlite3_step(res)) == SQLITE_ROW) {
        printf("%d | %s |  %s %s |  %s pourcent(s) \n",i, sqlite3_column_text(res, 0), sqlite3_column_text(res, 1), getConfig.devise, sqlite3_column_text(res, 3));
        i++;
    }

    sqlite3_finalize(res);
    sqlite3_close(db);
    return 0;
}

int displayAmountByAccountType(Config getConfig, sqlite3 *db, int id) {
    //char *err_msg = 0;
    sqlite3_stmt *res;
    int i;

    int rc = sqlite3_open(getConfig.db_name, &db);

    if (rc != SQLITE_OK) {

        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);

        return 1;
    }

    char *sql = "SELECT compte.*, client.prenom, client.nom FROM client, compte WHERE client.id = compte.id_client AND compte.id = @id";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

    if (rc == SQLITE_OK) {
        int id_default = sqlite3_bind_parameter_index(res, "@id");
        sqlite3_bind_int(res, id_default, id);
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }


    int step = sqlite3_step(res);
    if(step == SQLITE_DONE) {
        printf("/!\ No id has been associated.\n");
    }
    if (step == SQLITE_ROW) {
        printf("\n\nFirstname : %s , Lastname : %s \n\n", sqlite3_column_text(res, 5), sqlite3_column_text(res, 6));
        printf("ID |      SOLDE       | TAUX \n\n");
        printf("%s |      %s %s  |  %s pourcent(s) \n", sqlite3_column_text(res, 0), sqlite3_column_text(res, 1), getConfig.devise, sqlite3_column_text(res, 4));
    }

    sqlite3_finalize(res);
    sqlite3_close(db);
    return 0;
}


void adminMenu() {
    sqlite3 *db;
    Config getConfig = config_default;
    int choice;
    int id;
    do {
        printf("1 --- Display the sum of the amounts for all accounts \n");
        printf("2 --- View amount by account type \n");
        printf("3 --- Display the total amount of interest and by type of account to be paid by the bank at the end of the year \n");
        printf("4 --- Export application data \n");
        printf("5 --- Import application data \n");
        printf("6 --- Back to main menu ---\n\n");
        printf("Please choose : ");
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                 printf("--------------- DISPLAY SUM OF AMOUNTS FOR ALL ACCOUNTS ------------------------\n");
                displaySumAmount(getConfig, db);
                 printf("\n---------------------------------------\n");
                break;
            case 2:
                printf("-----------------  VIEW AMOUNT BY ACCOUNT TYPE  ----------------------\n\n");
                printf("Please enter an account identifiant : ");
                scanf("%d", &id);
                displayAmountByAccountType(getConfig, db, id);
                printf("\n---------------------------------------\n");
                break;
            case 3:

                break;
            case 4:

                break;
            case 5:

                break;
            case 6:
                menu();
                break;

            default:
                break;

        }

    } while(choice != 6);
}



void admin(char *pseudo) {
    printf("Welcome : %s\n\n", pseudo);
    adminMenu();
}


int logIn(Config getConfig, sqlite3 *db, char *name, char *password) {
    //char *err_msg = 0;
    sqlite3_stmt *res;

    int rc = sqlite3_open(getConfig.db_name, &db);

    if (rc != SQLITE_OK) {

        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);

        return 1;
    }

    char *sql = "SELECT * FROM client, admin WHERE LOWER(client.prenom) = LOWER(@name) AND client.id = admin.id_client";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

    if (rc == SQLITE_OK) {
        int id_default = sqlite3_bind_parameter_index(res, "@name");

        sqlite3_bind_text(res, id_default, name, -1, SQLITE_TRANSIENT);
    } else {

        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }
    int step = sqlite3_step(res);
    if( step == SQLITE_DONE) {
        return 2;
    }
    if (step == SQLITE_ROW) {
        if(strcmp(convertToLower(password), convertToLower((char *)sqlite3_column_text(res, 7))) == 0) {
            admin(name);
        } else {
            return 2;
        }
    }
    sqlite3_finalize(res);
    sqlite3_close(db);
    return 0;
}

void loginAdmin() {

    sqlite3 *db;
    Config getConfig = config_default;
    char name[255];
    char password[255];
    int cptCo = 0;
    printf("-----------------/ MY BANQUE : ADMIN /-----------------\n\n");
    printf("If you want to enter into the administration\n please, enter your name and your password\n\n");
    while(1) {
        if(cptCo == 3) {
            printf("To mutch attempt.\n -> Attempt max : 3\n\n");
            menu();
        } else {
            printf("Enter firstname : ");
            scanf("%s", name);
            printf("Enter password : ");
            scanf("%s", password);
            int getError = logIn(getConfig, db, name, password);
            if(getError == 2) {
                printf("\n --- > Authentication error\n\n");
                cptCo++;
            } else {
                break;
            }
        }

    }
}

void menu() {

    int choice;
    do {
        printf("-----------------/ MY BANQUE /-----------------\n\n");
        printf("1 --- Customer Management \n");
        printf("2 --- Account management \n");
        printf("3 --- Operations management \n");
        printf("4 --- Historic \n");
        printf("5 --- Administration\n");
        printf("6 --- Exit the program\n\n");
        printf("Please choose : ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:

                break;
            case 2:

                break;
            case 3:

                break;
            case 4:

                break;
            case 5:
                loginAdmin();
                break;
            case 6:
                exit(0);
                break;

            default:
                break;

        }
    } while(choice != 6);

}

int main(int argc, char* argv[]) {
    sqlite3 *db;
    int rc;
    Config getConfig = config_default;

    rc = sqlite3_open(getConfig.db_name, &db);

    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return(0);
    } else {
        menu();
    }
    sqlite3_close(db);

    return 0;
}
