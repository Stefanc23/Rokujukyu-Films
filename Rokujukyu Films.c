/*************************************************
 HEADERS DAN MACROS
*************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>

#define MAX_CHAR 24

/*************************************************
 DATA STRUCTURES
*************************************************/

typedef struct orders ///structure for each orders
{
    char movie[MAX_CHAR];
    int amount;
    int price;
    int payment;
} orders;

typedef struct node ///linked list node containing one order and two pointers
{
    orders* order;
    struct node* prev;
    struct node* next;
    ///Hashes for blockchain implementation
    unsigned long long int hash;
    unsigned long long int prevHash;
    unsigned long long int nextHash;
} node;

typedef struct list ///tree node as a linked list, each containing one or more linked list nodes and two pointers
{
    node* head;
    struct list* left;
    struct list* right;
    int nodes;
    ///Hashes for blockchain implementation
    unsigned long long int headHash;
    unsigned long long int leftHash;
    unsigned long long int rightHash;
    unsigned long long int parentHash;
} list;

typedef struct tree ///tree structure
{
    list* root;
    unsigned long long int rootHash;
    int lists;
} tree;

/*************************************************
 UTILITY FUNCTIONS
*************************************************/

void config()
{
    system("title \"ROKUJUKYU FILMS\" ");
    system("color F0");
}

void clrscr()
{
    system("@cls||clear");
}

/*************************************************
 MISC FUNCTIONS
*************************************************/

void capitalizeEachWord(char* arr, int slen)
{
    int i;
    arr[0] = toupper(arr[0]);
    for(i=1; i<slen; ++i)
    {
        if(arr[i] == ' ')
            continue;
        if(arr[i-1] == ' ')
            arr[i] = toupper(arr[i]);
        else
            arr[i] = tolower(arr[i]);
    }
}

unsigned char *ordersStruct2str (orders* O)
{
    size_t len = 0;
    len = snprintf (NULL, len, "%s,%d,%d,%d", O->movie, O->amount, O->price, O->payment);

    char *ostr = calloc (1, sizeof *ostr * len + 1);
    if (!ostr) {
        fprintf (stderr, "%s() error: virtual memory allocation failed.\n", __func__);
    }

    if (snprintf (ostr, len + 1, "%s,%d,%d,%d", O->movie, O->amount, O->price, O->payment) > len + 1)
    {
        fprintf (stderr, "%s() error: snprintf returned truncated result.\n", __func__);
        return NULL;
    }

    return ostr;
}

unsigned long long int hash(unsigned char *str) ///polynomial hashing with djb2 hash function
{
    unsigned long long int result = 5381;
    int c;

    while (c = *str++)
        result = ((result << 5) + result) + c; ///hash * 33 + c

    return result;
}

/*************************************************
 DATA STRUCTURE FUNCTIONS
*************************************************/

void initTree(tree* T)
{
    T->root = NULL;
    T->lists = 0;
    return;
}

orders* newOrder(char movie[], int amount, int price, int payment)
{
    orders* o = (orders*)malloc(sizeof(orders));
    strcpy(o->movie, movie);
    o->amount = amount;
    o->price = price;
    o->payment = payment;
    return o;
}

node* newNode(orders* o)
{
    node* n = (node*)malloc(sizeof(node));
    n->order = o;
    n->prev = NULL;
    n->next = NULL;
    n->hash = hash(ordersStruct2str(n->order));
    n->prevHash = 0;
    n->nextHash = 0;
    return n;
}

list* newList(node* n)
{
    list* l = (list*)malloc(sizeof(list));
    l->head = n;
    l->headHash = n->hash;
    l->left = NULL;
    l->right = NULL;
    l->nodes = 1;
    l->leftHash = 0;
    l->rightHash = 0;
    l->parentHash = 0;
    return l;
}

node* searchInList(node* head_ref, int target)
{
    int i = 1;
    node* nptr = head_ref;
    while(nptr->next != NULL)
        nptr = nptr->next;
    while(i < target)
    {
        nptr = nptr->prev;
        i++;
    }
    return nptr;
}

list* searchInTree(list* lptr, char movie[])
{
    if(lptr == NULL || strcmp(lptr->head->order->movie, movie)==0)
        return lptr;
    if(strcmp(lptr->head->order->movie, movie) < 0)
        return searchInTree(lptr->right, movie);
    return searchInTree(lptr->left, movie);
}

list* searchParentInTree(list* parent, list* lptr, list* target)
{
    if(lptr == target)
        return parent;
    if(strcmp(lptr->head->order->movie, target->head->order->movie) < 0)
        return searchParentInTree(lptr, lptr->right, target);
    return searchParentInTree(lptr, lptr->left, target);
}

void insertNode(list* lptr, node* n)
{
    node* nptr = lptr->head;
    while(nptr->next != NULL)
        nptr = nptr->next;
    nptr->next = n;
    n->prev = nptr;
    n->prev->nextHash = n->hash;
    n->prevHash = n->prev->hash;
    lptr->nodes++;
}

list* insertList(list* lptr, tree* T, char movie[], int amount, int price, int payment)
{
    if(lptr == T->root && T->root == NULL)
    {
        T->root = newList(newNode(newOrder(movie, amount, price, payment)));
        T->rootHash = T->root->headHash;
    }
    else
    {
        if(lptr == NULL) {
            T->lists++;
            return newList(newNode(newOrder(movie, amount, price, payment)));
        }
        if(strcmp(lptr->head->order->movie, movie) > 0)
            lptr->left = insertList(lptr->left, T, movie, amount, price, payment);
        else if(strcmp(lptr->head->order->movie, movie) < 0)
            lptr->right = insertList(lptr->right, T, movie, amount, price, payment);
        else if(strcmp(lptr->head->order->movie, movie) == 0)
            insertNode(lptr, newNode(newOrder(movie, amount, price, payment)));
    }
}

void verifyNodesInList(node* head_ref, list* l)
{
    char status[10];
    node* nptr = head_ref;
    while(nptr->next != NULL)
        nptr = nptr->next;
    if(nptr->hash == nptr->prev->nextHash)
        strcpy(status, "verified");
    else
        strcpy(status, "changed");
    printf(" = %-24s = %6d = $%4d = $%6d = %8s =\n", nptr->order->movie, nptr->order->amount, nptr->order->price, nptr->order->payment, status);
    while(nptr != head_ref)
    {
        nptr = nptr->prev;
        if(nptr == head_ref)
        {
            if(nptr->hash == l->headHash)
                strcpy(status, "verified");
            else
                strcpy(status, "changed");
        }
        else
        {
            if(nptr->hash == nptr->prev->nextHash)
                strcpy(status, "verified");
            else
                strcpy(status, "changed");
        }

        printf(" = %24c = %6d = $%4d = $%6d = %8s =\n", ' ', nptr->order->amount, nptr->order->price, nptr->order->payment, status);
    }
}

void verifyTree(list* parent, list* lptr, tree* T)
{
    if(lptr != NULL)
    {
        verifyTree(lptr, lptr->left, T);
        if(lptr->head->next == NULL)
        {
            char status[10];
            if(lptr == T->root)
            {
                if(T->root->left == NULL && T->root->right == NULL)
                {
                    if(T->root->head->hash == T->rootHash)
                        strcpy(status, "verified");
                    else
                        strcpy(status, "changed");
                }
                else
                {
                    if(lptr->left == NULL)
                    {
                        if(lptr->right->parentHash == lptr->head->hash)
                            strcpy(status, "verified");
                        else
                            strcpy(status, "changed");
                    }
                    else
                    {
                        if(lptr->left->parentHash == lptr->head->hash)
                            strcpy(status, "verified");
                        else
                            strcpy(status, "changed");
                    }
                }
            }
            else
            {
                if(parent->left == lptr)
                {
                    if(parent->leftHash == lptr->head->hash)
                        strcpy(status, "verified");
                    else
                        strcpy(status, "changed");
                }
                else if(parent->right == lptr)
                {
                    if(parent->rightHash == lptr->head->hash)
                        strcpy(status, "verified");
                    else
                        strcpy(status, "changed");
                }
            }
            printf(" = %-24s = %6d = $%4d = $%6d = %8s =\n", lptr->head->order->movie, lptr->head->order->amount, lptr->head->order->price, lptr->head->order->payment, status);
        }
        else
            verifyNodesInList(lptr->head, lptr);
        verifyTree(lptr, lptr->right, T);
    }
}

void printList(node* head_ref)
{
    node* nptr = head_ref;
    while(nptr->next != NULL)
        nptr = nptr->next;
    printf(" = %-24s = %6d = $%4d = $%6d =\n", nptr->order->movie, nptr->order->amount, nptr->order->price, nptr->order->payment);
    while(nptr != head_ref)
    {
        nptr = nptr->prev;
        printf(" = %24c = %6d = $%4d = $%6d =\n", ' ', nptr->order->amount, nptr->order->price, nptr->order->payment);
    }
}

void inorderTraversal(list* lptr)
{
    if(lptr != NULL)
    {
        inorderTraversal(lptr->left);
        if(lptr->head->next == NULL)
            printf(" = %-24s = %6d = $%4d = $%6d =\n", lptr->head->order->movie, lptr->head->order->amount, lptr->head->order->price, lptr->head->order->payment);
        else
            printList(lptr->head);
        inorderTraversal(lptr->right);
    }
}

/*************************************************
 MENU FUNCTIONS
*************************************************/

void viewData(tree* T)
{
    clrscr();
    printf(" =======================================================\n");
    printf(" = %-24s = %s = %s = %s =\n", "Movie Title", "Amount", "Price", "Payment");
    printf(" =======================================================\n");
    if(T->root == NULL)
        printf(" =                    Data is empty!                   =\n");
    else
    {
        inorderTraversal(T->root);
    }
    printf(" =======================================================\n ");
    return;
}

void addData(tree* T)
{
    char movie[MAX_CHAR];
    int amount, price, payment;
    do
    {
        clrscr();
        printf(" Input movie name [1-%d characters] : ", MAX_CHAR);
        fflush(stdin);
        scanf("%[^\n]s", movie);
        if(movie[strlen(movie)-1] == ' ')
            movie[strlen(movie)-1] = '\0';
    }
    while(strlen(movie) < 0 || strlen(movie) > MAX_CHAR);
    capitalizeEachWord(movie, strlen(movie));
    do
    {
        printf("\n Input amount [1-10] : ");
        scanf("%d", &amount);
    }
    while(amount < 1 || amount > 10);
    do
    {
        printf("\n Input price per piece [$1-100] : $");
        scanf("%d", &price);
    }
    while(price < 1 || price > 100);
    do
    {
        printf("\n Input payment [min $%d] : $", price*amount);
        scanf("%d", &payment);
    }
    while(payment < price*amount);
    insertList(T->root, T, movie, amount, price, payment);
    list* L = searchInTree(T->root, movie);
    if(T->root->left != NULL || T->root->right != NULL)
    {
        list* parent = searchParentInTree(T->root, T->root, L);
        if(parent->left == L)
            parent->leftHash = L->headHash;
        else if(parent->right == L)
            parent->rightHash = L->headHash;
        L->parentHash = parent->headHash;
    }
    printf("\n\n Data input success!\n ");
    return;
}

void updateData(tree* T)
{
    char movie[MAX_CHAR];
    viewData(T);
    if(T->root != NULL)
    {
        do
        {
            printf("\n Input the movie title of the transaction that you want to update [1-%d characters] : ", MAX_CHAR);
            fflush(stdin);
            scanf("%[^\n]s", movie);
            if(movie[strlen(movie)-1] == ' ')
                movie[strlen(movie)-1] = '\0';
        }
        while(strlen(movie) < 1 || strlen(movie) > MAX_CHAR);
        capitalizeEachWord(movie, strlen(movie));
        list* target = searchInTree(T->root, movie);
        if(target != NULL)
        {
            node* targetNode = target->head;
            if(targetNode->next != NULL)
            {
                int targetIdx;
                do
                {
                    printf("\n Input which transaction for the movie %s that you want to change [1-%d] : ", target->head->order->movie, target->nodes);
                    scanf("%d", &targetIdx);
                }
                while(targetIdx < 1 || targetIdx > target->nodes);
                targetNode = searchInList(target->head, targetIdx);
            }
            do
            {
                printf("\n Input new amount [1-10] : ");
                scanf("%d", &targetNode->order->amount);
            }
            while(targetNode->order->amount < 1 || targetNode->order->amount > 10);
            do
            {
                printf("\n Input new price per piece [$1-100] : $");
                scanf("%d", &targetNode->order->price);
            }
            while(targetNode->order->price < 1 || targetNode->order->price > 100);
            do
            {
                printf("\n Input new payment [min $%d] : $", targetNode->order->price * targetNode->order->amount);
                scanf("%d", &targetNode->order->payment);
            }
            while(targetNode->order->payment < targetNode->order->price * targetNode->order->amount);
            targetNode->hash = hash(ordersStruct2str(targetNode->order));
            printf("\n\n Update success!\n\n ");
        }
        else
            printf("\n There is no movie with that title!\n\n ");
    }
}

void verifyData(tree* T)
{
    clrscr();
    printf(" ==================================================================\n");
    printf(" = %-24s = %s = %s = %s =  %s  =\n", "Movie Title", "Amount", "Price", "Payment", "Status");
    printf(" ==================================================================\n");
    if(T->root == NULL)
        printf(" =                          Data is empty!                        =\n");
    else
    {
        verifyTree(T->root, T->root, T);
    }
    printf(" ==================================================================\n ");
    return;
}

void mainMenu()
{
    printf(" ==================================================\n");
    printf(" =          WELCOME  TO  \"ROKUJUKYU FILMS\"        =\n");
    printf(" ==================================================\n\n\n");
    printf(" 1. View Transactions\n");
    printf(" 2. Add Transaction\n");
    printf(" 3. Update Transaction\n");
    printf(" 4. Verify Transaction Data\n");
    printf(" 0. Exit\n\n");
    printf(" >> Your choice: ");
}

/*************************************************
 MAIN FUNCTION
*************************************************/

void main()
{
    int opt;
    tree* T = (tree*)malloc(sizeof(tree));
    initTree(T);
    config();
    do
    {
        clrscr();
        mainMenu();
        scanf("%d", &opt);
        switch(opt)
        {
            case 1:
                viewData(T);
                getch();
                break;

            case 2:
                addData(T);
                getch();
                break;

            case 3:
                updateData(T);
                getch();
                break;

            case 4:
                verifyData(T);
                getch();
                break;

            default:
                break;
        }
    }
    while(opt != 0);
}
