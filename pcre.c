#include "gatekeeper.h"

/*
 * pcre_pcre_list_add(): adds a value to the end of a singly linked list
 * returns:    -1 on error, 0 on success
 * notes:      value must point to space allocated on the heap
 */
int pcre_list_add(pcre *re, pcre_list_t **head)
{
    pcre_list_t *i = NULL, *ptr = NULL;
    /* does head even point to anything? */
    if (head == NULL) {
        return -1;
    }
    i = (pcre_list_t *)calloc(1, sizeof(pcre_list_t));
    if (!i) {
        return -1;
    }
    i->re = re;
    i->next = NULL;
    /* is the list currently empty?  if so, the new node is the first
       and only element, assign it to head and return */
    if (*head == NULL) {
        *head = i;
        return 0;
    }
    ptr = *head;
    while (ptr) {
        /* are we at the end of the list?  if so, insert new node
           at the end */
        if (ptr->next == NULL) {
            ptr->next = i;
            break;
        }
        ptr = ptr->next;
    }
    return 0;
}

/*
 * free_list(): iterates a singly linked list, free()'ing the value,
 *              then the node itself.
 * returns:     none
 */
void free_list(pcre_list_t *list)
{
    pcre_list_t *ptr, *tmp;
    if (list == NULL) {
        return;
    }
    ptr = list;
    while (ptr) {
        tmp = ptr->next;
        if (ptr->re)
            free(ptr->re);
        free(ptr);
        ptr = tmp;
    }
}

/*
 * take supplied filename and read it line-by-line, and compile
 * the line as a pcre.  compiled pcres are added to a singly-linked
 * list that we'll use later to match traffic against
 */

pcre_list_t *parse_pcre_inputs(const char *fname)
{
    FILE *f;
    char line[1024];
    pcre *re;
    pcre_list_t *pcre_list = NULL;
    const char *error;
    int erroffset;
    int linenum;
    
    if ((f = fopen(fname, "r")) == NULL) {
        Log("Error opening %s for reading: %s\n", fname, strerror(errno));
        return NULL;
    }
    Log("Parsing pcre inputs from %s... ", fname);
    linenum = 1;
    while (fgets(line, sizeof(line), f) != NULL) {
        re = pcre_compile(line, 0, &error, &erroffset, NULL);
        if (re == NULL) {
            /* compilation failed, bail out */
            Log("PCRE compilation failed on line %d at offset %d: %s\n",
                linenum, erroffset, error);
            return NULL;
        }
        /* compilation succeeded, add to linked list */
        pcre_list_add(re, &pcre_list);
        linenum++;
    }
    fclose(f);
    Log("Done.\nParsed %d pcre inputs.\n", linenum);

    return pcre_list;
}

/*
 * iterates the list of compiled pcres specified by pcre_list,
 * checking for any matches against the supplied buffer.
 * returns 1 on the first match, returns 0 if no matches are found.
 */

int check_for_match(pcre_list_t *p, char *buf, int num_bytes)
{
    int rc;
    pcre_list_t *ptr;

    for (ptr = p; ptr; ptr = ptr->next) {
        /* check each compiled regex against the buffer */
        rc = pcre_exec(ptr->re, NULL, buf, num_bytes, 0, 0, NULL, 0);
        if (rc < 0) {
            /* either there was no match or an error occured */
            if (debugging) {
                Log("pcre_exec() returned %d: ", rc);
                switch (rc) {
                case PCRE_ERROR_NOMATCH:
                    Log("String did not match the pattern\n");
                    break;
                case PCRE_ERROR_NULL:
                    Log("Something was null\n");
                    break;
                case PCRE_ERROR_BADOPTION:
                    Log("A bad option was passed\n");
                    break;
                case PCRE_ERROR_BADMAGIC:
                    Log("Magic number bad (compiled re corrupt?)\n");
                    break;
                case PCRE_ERROR_UNKNOWN_NODE:
                    Log("Something kooky in the compiled re\n");
                    break;
                case PCRE_ERROR_NOMEMORY:
                    Log("Ran out of memory\n");
                    break;
                default:
                    Log("Unknown error\n");
                    break;
                }
            }
            continue;
        } else {
            if (debugging) {
                Log("Found matching pcre in buffer.\n");
            }
            return 1;
        }
    }
    return 0;
}
