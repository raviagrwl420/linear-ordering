#include<ilp_solver_c.h>

#include<stdlib.h>

/* Define a struct to represent ranking data */
typedef struct {
   int size;
   int num_vars;
   int** matrix;
} RANKING;

/* Define a struct to represent a cut */
typedef struct {
    int i;
    int j;
    int k;
    double val;
    double objpar;
} CUT;

/* Define a struct to represent a cut array */
typedef struct {
    CUT* array;
    int used;
    int size;
} CUT_ARRAY;

/* Define a struct to represent user callback handle */
typedef struct {
    /* Ranking that we are working on */
    RANKING r;
    /* Pointer to an array that stores all cuts */
    CUT_ARRAY allcuts;
    /* Pointer to store the env */
    CPXENVptr envptr;
    /* Pointer to store node lp */
    CPXLPptr nodelp;
    /* Pointer to store the copy of node lp */
    CPXLPptr copy;
    /* Array pointers to store the bases */
    int* cstat;
    int* rstat;
    /* Array pointer to store the values */
    double* x;
} USER_HANDLE;

enum CRITERIA {VIOLATION, OBJ_FUNCTION_PARALLELISM};

/* epsilon used for violation of cuts */
#define EPS 1e-6

/* rhs for dicycle cuts */
#define RHS 2.0
#define DICYCLE_CUT_COEFF 1.0

/* number of non zeros for constraints */
#define NUM_NNZ_2 2
#define NUM_NNZ_3 3

/* define cut selection related constants */
#define MAX_VIOLATED_CUTS 10000
#define SELECTION_BY_CRITERIA 5000
#define EVALUATE_COUNT 500
#define SELECTION_BY_EVALUATION 200
#define MAX_CUTS 200

/* define runtime params */
CPXLONG MAX_ITERATIONS = 100; 
CPXLONG MAX_ITERATIONS_DEFAULT = 9223372036700000000;

/* define params for branching */
CPXLONG MAX_ITERATIONS_BRANCH = 1000;

/* define a function to initialize a new cut array */
void initCutArray(CUT_ARRAY* cuts, int initialSize) {
    cuts->array = (CUT*) malloc (initialSize * sizeof(CUT));
    cuts->used = 0;
    cuts->size = initialSize;
}

/* define a function to insert element in the cut array */
void insertCutArray(CUT_ARRAY* cuts, CUT cut) {
    if (cuts->used == cuts->size) {
        cuts->size *= 2;
        cuts->array = (CUT*) realloc (cuts->array, cuts->size * sizeof(CUT));
    }
    cuts->array[cuts->used++] = cut;
}

/* define a function to get an element from the array */
CUT getCut(CUT_ARRAY* cuts, int index) {
    return cuts->array[index];
}

/* define a function to free the cut array */
void freeCutArray(CUT_ARRAY* cuts) {
    free(cuts->array);
    cuts->array = NULL;
    cuts->used = cuts->size = 0;
}

/* define a compare function for cuts */
int compareCuts(const void* c1, const void* c2) {
    return ((CUT *) c2)->val - ((CUT *) c1)->val;
}

/* define a function to sort the cuts based on val */
void sort(CUT_ARRAY* cuts) {
    qsort(cuts->array, cuts->used, sizeof(CUT), compareCuts);
}

/* define a function to generate all the dicycle cuts */
void generateCuts(CUT_ARRAY* cuts, RANKING r) {
    int size = r.size;
    int** matrix = r.matrix;

    int initialSize = (size * (size-1) * (size-2)) / 3;
    initCutArray(cuts, initialSize);

    int c = 0;
    for (int i = 0; i < size; i++) {
        for (int j = i + 1; j < size; j++) {
            for (int k = i + 1; k < size; k++) {
                if (j != k) {
                    int objpar = matrix[i][j] + matrix[j][k] + matrix[k][i];
                    CUT cut = {.i = i, .j = j, .k = k, .val = 0, .objpar = objpar};
                    insertCutArray(cuts, cut);
                    c++;
                }
            }
        }
    }
}

/* define a function to calculate indices */
static int get_ind(RANKING r, int i, int j) {
    return i * r.size + j;
}

/* define a function to check the violation of a cut */
static bool is_violated(RANKING r, double* x, CUT cut) {
    return x[get_ind(r, cut.i, cut.j)] + x[get_ind(r, cut.j, cut.k)] + x[get_ind(r, cut.k, cut.i)] > RHS + EPS;
}

/* define a function to get the violation of a cut */
static double get_violation(RANKING r, double* x, CUT cut) {
    return x[get_ind(r, cut.i, cut.j)] + x[get_ind(r, cut.j, cut.k)] + x[get_ind(r, cut.k, cut.i)] - RHS; 
}

/* define a function to get violated cuts */
void getViolatedCuts(USER_HANDLE u, double* x, CUT_ARRAY* violatedCuts, int max) {
    int initialSize = max + 1; // HACK!
    initCutArray(violatedCuts, initialSize);

    int c = 0;
    for (int i = 0; i < u.allcuts.used; i++) {
        if (c >= max) break;

        CUT cut = getCut(&(u.allcuts), i);
        if (is_violated(u.r, x, cut)) {
            insertCutArray(violatedCuts, cut);
            c++;
        }
    }
}

/* define a function to fill criteria values */
void fillCriteriaValues(USER_HANDLE u, double* x, CUT_ARRAY* violatedCuts) {
    CRITERIA c = OBJ_FUNCTION_PARALLELISM;

    switch (c) {
        case VIOLATION:
            for (int i = 0; i < violatedCuts->used; i++) {
                CUT* cut = &(violatedCuts->array[i]);
                cut->val = get_violation(u.r, x, *cut);
            }
            break;
        case OBJ_FUNCTION_PARALLELISM:
            for (int i = 0; i < violatedCuts->used; i++) {
                CUT* cut = &(violatedCuts->array[i]);
                cut->val = cut->objpar;
            }
            break;
        default:
            printf("Unsuported Criteria\n");
    }
}

/* define a function to evaluate a cut */
static double evaluate_cut(CPXCENVptr env, CPXLPptr lp, USER_HANDLE u, CUT* cutptr) {
    RANKING r = u.r;
    CPXENVptr envptr = u.envptr;
    int status = 0;

    /* Get the cut reference */
    CUT cut = *cutptr;
    
    /* Get the objective value */
    double preobj;
    status = CPXXgetobjval(env, lp, &preobj);

    /* Get the number of rows */
    int nRows = CPXXgetnumrows(env, lp);

    /* Prepare the cut */
    CPXNNZ rmatbeg[] = {0};
    int rmatind[] = {get_ind(r, cut.i, cut.j), get_ind(r, cut.j, cut.k), get_ind(r, cut.k, cut.i)};
    double rmatval[] = {DICYCLE_CUT_COEFF, DICYCLE_CUT_COEFF, DICYCLE_CUT_COEFF};
    double rhs[] = {RHS};
    char senses[] = {'L'};

    /* Add the cut */
    status = CPXXaddrows(env, lp, 0, 1, NUM_NNZ_3, rhs, senses, rmatbeg, rmatind, rmatval, NULL, NULL);
    
    /* Set the maximum number of iterations */
    status = CPXXsetlongparam(envptr, CPX_PARAM_ITLIM, MAX_ITERATIONS);
    /* Run dual opt */
    status = CPXXdualopt(env, lp);
    /* Reset the maximum number of iterations */
    status = CPXXsetlongparam(envptr, CPX_PARAM_ITLIM, MAX_ITERATIONS_DEFAULT);

    /* Get the solution again */
    double postobj;
    status = CPXXgetobjval(env, lp, &postobj);

    /* Delete row and use primal opt */
    status = CPXXdelrows(env, lp, nRows, nRows);
    status = CPXXprimopt(env, lp);

    cutptr->val = preobj - postobj;

    return cutptr->val;
}

void getEvaluatedCuts(CPXCENVptr env, CPXLPptr lp, USER_HANDLE u, CUT_ARRAY violatedCuts, CUT_ARRAY* evaluatedCuts) {
    int initialSize = SELECTION_BY_EVALUATION;
    initCutArray(evaluatedCuts, initialSize);

    int c = 0;
    for (int i = 0; i < (violatedCuts.used < EVALUATE_COUNT ? violatedCuts.used : EVALUATE_COUNT); i++) {
        if (c >= SELECTION_BY_EVALUATION) break;

        CUT cut = getCut(&violatedCuts, i);
        cut.val = evaluate_cut(env, lp, u, &cut);
        if (cut.val > EPS) {
            insertCutArray(evaluatedCuts, cut);
            c++;
        }
        // printf("Cuts Evaluated: %d of %d Selected: %d\n", i, violatedCuts.used, c);
    }
}

static int CPXPUBLIC cutcallback(CPXCENVptr env, void *cbdata, int wherefrom,
    void *cbhandle, int *useraction_p) {
    /* Extract data from user handle */
    USER_HANDLE u = *((USER_HANDLE*) cbhandle);
    
    RANKING r = u.r;
    int size = r.size;
    int num_vars = r.num_vars;

    CUT_ARRAY allcuts = u.allcuts;
    
    CPXLPptr nodelp = u.nodelp;
    CPXLPptr copy = u.copy;
    int* cstat = u.cstat;
    int* rstat = u.rstat;
    double* x = u.x;

    /* Initialize a new array to hold the values */
    x = (double *) malloc (num_vars * sizeof (double));

    /* No changes made yet */
    int status = 0;
    *useraction_p = CPX_CALLBACK_DEFAULT;

    /* Temporary variables to store the data */
    double objval;

    /* Only when CPLEX has given up on generating cuts */
    if (wherefrom == CPX_CALLBACK_MIP_CUT_LAST) {
        /* Get the node lp */
        CPXXgetcallbacknodelp(env, cbdata, wherefrom, &nodelp);

        /* Get the number of columns and rows in node lp */
        int nCols = CPXXgetnumcols(env, nodelp);
        int nRows = CPXXgetnumrows(env, nodelp);

        /* Clone the problem */
        copy = CPXXcloneprob(env, nodelp, &status);

        /* Get the base */
        cstat = (int *) malloc (nCols * sizeof (int));
        rstat = (int *) malloc (nRows * sizeof (int));
        status = CPXXgetbase(env, nodelp, cstat, rstat);

        /* Copyt the base to the cloned problem */
        status = CPXXcopybase(env, copy, cstat, rstat);

        /* Run dual opt on the cloned problem */
        status = CPXXdualopt(env, copy);

        /* Get the objective value */
        status = CPXXgetobjval(env, nodelp, &objval);

        /* Get the current solution */
        status = CPXXgetcallbacknodex(env, cbdata, wherefrom, x, 0, num_vars - 1);
        if (status != 0)
            return status;

        /* Get the violated cuts */
        CUT_ARRAY violatedCuts;
        getViolatedCuts(u, x, &violatedCuts, MAX_VIOLATED_CUTS);

        /* Fill the criteria values */
        fillCriteriaValues(u, x, &violatedCuts);

        /* Sort by the criteria values */
        sort(&violatedCuts);

        /* Get evaluated cuts */
        CUT_ARRAY evaluatedCuts;
        getEvaluatedCuts(env, copy, u, violatedCuts, &evaluatedCuts);

        /* Sort evaluated cuts */
        sort(&evaluatedCuts);

        int c = 0;
        for (int i = 0; i < (evaluatedCuts.used < MAX_CUTS ? MAX_CUTS : evaluatedCuts.used); i++) {
            CUT cut;
            if (i < evaluatedCuts.used) {
                cut = getCut(&evaluatedCuts, i);
            } else {
                if (i - evaluatedCuts.used < violatedCuts.used) {
                    cut = getCut(&violatedCuts, i - evaluatedCuts.used);  
                } else {
                    break;
                }
            }

            if (is_violated(r, x, cut)) {
                int cutind[NUM_NNZ_3] = {get_ind(r, cut.i, cut.j), get_ind(r, cut.j, cut.k), get_ind(r, cut.k, cut.i)};
                double cutval[NUM_NNZ_3] = {DICYCLE_CUT_COEFF, DICYCLE_CUT_COEFF, DICYCLE_CUT_COEFF};

                if (c < MAX_CUTS) {
                    status = CPXXcutcallbackadd(env, cbdata, wherefrom, NUM_NNZ_3, 2.0,
                                                'L', cutind, cutval, CPX_USECUT_FORCE);
                    c++;
                }
            }
        }

        /* Free cut array */
        freeCutArray(&violatedCuts);
        freeCutArray(&evaluatedCuts);
        
        *useraction_p = CPX_CALLBACK_SET;
    }

    /* Free Memory */
    free(x);
    free(cstat);
    free(rstat);

    return status;
}

static int CPXPUBLIC lazycallback(CPXCENVptr env, void *cbdata, int wherefrom,
    void *cbhandle, int *useraction_p) {

    // printf("Lazy Callback!\n");

    USER_HANDLE u = *((USER_HANDLE*) cbhandle);
    RANKING r = u.r;
    int size = r.size;
    int num_vars = r.num_vars;
    int** matrix = r.matrix;

    int status;
    double x[num_vars];

    status = CPXXgetcallbacknodex(env, cbdata, wherefrom, x, 0, num_vars - 1);
    if (status != 0)
        return status;

    int c = 0;
    for (int i = 0; i < u.allcuts.size; i++) {
        CUT cut = getCut(&(u.allcuts), i);

        if (is_violated(r, x, cut)) {
            int cutind[NUM_NNZ_3] = {get_ind(r, cut.i, cut.j), get_ind(r, cut.j, cut.k), get_ind(r, cut.k, cut.i)};
            double cutval[NUM_NNZ_3] = {DICYCLE_CUT_COEFF, DICYCLE_CUT_COEFF, DICYCLE_CUT_COEFF};

            if (c < MAX_CUTS) {
                status = CPXXcutcallbackadd(env, cbdata, wherefrom, NUM_NNZ_3, 2.0,
                                            'L', cutind, cutval, CPX_USECUT_FORCE);
                c++;
            }
        }
    }

    *useraction_p = CPX_CALLBACK_SET;

    // printf("Cuts Applied: %d\n", c);

    return status;
}

static int addPrecedenceConstraints(CPXENVptr env, CPXLPptr lp, int size, int** matrix) {
    int num_const = (size * size - size) / 2;

    double rhs[num_const];
    char sense[num_const];

    int num_nz = 2;
    CPXNNZ rmatbeg[num_const];
    int rmatind[num_const * num_nz];
    double rmatval[num_const * num_nz];

    int c = 0;
    for (int i = 0; i < size; i++) {
        for (int j = i + 1; j < size; j++) {
            rhs[c] = 1.0;
            sense[c] = 'E';
            rmatbeg[c] = c * num_nz;

            rmatind[rmatbeg[c]] = i * size + j;
            rmatind[rmatbeg[c] + 1] = j * size + i;

            rmatval[rmatbeg[c]] = 1;
            rmatval[rmatbeg[c] + 1] = 1;
            c++;
        }
    }

    return CPXXaddrows(env, lp, 0, num_const, num_const * num_nz, rhs, sense,
                      rmatbeg, rmatind, rmatval, NULL, NULL);
}

static int addDicycleConstraints(CPXENVptr env, CPXLPptr lp, int size, int** matrix) {
    int num_const = (size * (size - 1) * (size - 2)) / 3;

    double rhs[num_const];
    char sense[num_const];

    int num_nz = 3;
    CPXNNZ rmatbeg[num_const];
    int rmatind[num_const * num_nz];
    double rmatval[num_const * num_nz];

    int c = 0;
    for (int i = 0; i < size; i++) {
        for (int j = i + 1; j < size; j++) {
            for (int k = i + 1; k < size; k++) {
                if (j != k) {
                    rhs[c] = 2.0;
                    sense[c] = 'L';
                    rmatbeg[c] = c * num_nz;

                    rmatind[rmatbeg[c]] = i * size + j;
                    rmatind[rmatbeg[c] + 1] = j * size + k;
                    rmatind[rmatbeg[c] + 2] = k * size + i;

                    rmatval[rmatbeg[c]] = 1;
                    rmatval[rmatbeg[c] + 1] = 1;
                    rmatval[rmatbeg[c] + 2] = 1;

                    c++;
                }
            }
        }
    }

    return CPXXaddlazyconstraints(env, lp, num_const, num_const * num_nz,
                     rhs, sense, rmatbeg, rmatind, rmatval, NULL);

    // return CPXXaddrows(env, lp, 0, num_const, num_const * num_nz,
    //                   rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
}

/* Implement helper methods for branching based on node position */
double eval_branch_candidate (CPXCENVptr env, CPXLPptr lp, USER_HANDLE u, int node, int position, int up) {
    RANKING r = u.r;
    int size = r.size;

    CPXENVptr envptr = u.envptr;
    
    int status = 0;

    /* Get the objective value */
    double preobj;
    status = CPXXgetobjval(env, lp, &preobj);
    // printf("Pre Obj: %f\n", preobj);

    /* Get the number of rows */
    int nRows = CPXXgetnumrows(env, lp);

    /* Prepare the cut */
    CPXNNZ rmatbeg[] = {0, size - 1};
    int rmatind[2 * (size - 1)];
    double rmatval[2 * (size - 1)];
    
    int k = 0;
    for (int j = 0; j < size; j++) {
        if (j != node) {
            rmatind[k] = get_ind(r, j, node);
            rmatval[k] = 1;
            rmatind[size - 1 + k] = get_ind(r, node, j);
            rmatval[size - 1 + k] = 1;
            k++;
        }
    }
    double rhs[] = {position, size - 1 - position};
    char senses[] = {'L', 'L'};
    if (up) {
        rhs[0] = position;
        rhs[1] = size - 1 - position;
        senses[0] = 'G';
        senses[1] = 'G';
    }

    /* Add the cut */
    status = CPXXaddrows(env, lp, 0, 1, NUM_NNZ_3, rhs, senses, rmatbeg, rmatind, rmatval, NULL, NULL);
    
    /* Set the maximum number of iterations */
    status = CPXXsetlongparam(envptr, CPX_PARAM_ITLIM, MAX_ITERATIONS_BRANCH);
    /* Run dual opt */
    status = CPXXdualopt(env, lp);
    /* Reset the maximum number of iterations */
    status = CPXXsetlongparam(envptr, CPX_PARAM_ITLIM, MAX_ITERATIONS_DEFAULT);

    /* Get the solution again */
    double postobj;
    status = CPXXgetobjval(env, lp, &postobj);
    // printf("Post Obj: %f\n", postobj    );

    /* Delete row and use primal opt */
    status = CPXXdelrows(env, lp, nRows, nRows);
    status = CPXXprimopt(env, lp);

    return preobj - postobj;
}

static int CPXPUBLIC branchcallback (CPXCENVptr env, void *cbdata, int wherefrom,
           void *cbhandle, int type, CPXDIM sos, int nodecnt, CPXDIM bdcnt,
           const CPXDIM *nodebeg, const CPXDIM *indices, const char *lu,
           const double *bd, const double *nodeest, int *useraction_p) {

    USER_HANDLE u = *((USER_HANDLE*) cbhandle);
    RANKING r = u.r;
    int size = r.size;
    int num_vars = r.num_vars;
    int** matrix = r.matrix;

    CPXLPptr nodelp = u.nodelp;
    CPXLPptr copy = u.copy;
    int* cstat = u.cstat;
    int* rstat = u.rstat;
    double* x = u.x;

    int status = 0;
    double objval;

    /* Get the node lp */
    CPXXgetcallbacknodelp(env, cbdata, wherefrom, &nodelp);

    /* Get the number of columns and rows in node lp */
    int nCols = CPXXgetnumcols(env, nodelp);
    int nRows = CPXXgetnumrows(env, nodelp);

    /* Clone the problem */
    copy = CPXXcloneprob(env, nodelp, &status);

    /* Get the base */
    cstat = (int *) malloc (nCols * sizeof (int));
    rstat = (int *) malloc (nRows * sizeof (int));
    status = CPXXgetbase(env, nodelp, cstat, rstat);

    /* Copyt the base to the cloned problem */
    status = CPXXcopybase(env, copy, cstat, rstat);

    /* Run dual opt on the cloned problem */
    status = CPXXdualopt(env, copy);

    /* Get the objective value */
    status = CPXXgetobjval(env, nodelp, &objval);

    double max_down = 0;
    double max_up = 0;
    for (int i = 0; i < size; i++) {
        double down = eval_branch_candidate(env, copy, u, 0, size/2, false);
        double up = eval_branch_candidate(env, copy, u, 0, size/2, true);
        // printf("Node: %2d\t down: %2.4f\t up: %2.4f\n", i, down, up);

        if (up > max_up) max_up = up;
        if (down > max_down) max_down = down;
    }
    printf("Max Down: %f Max Up: %f\n", max_down, max_up);

    for (int i = 0; i < nodecnt; i++) {
        CPXCNT random;
        CPXXbranchcallbackbranchbds(env, cbdata, wherefrom, 1, &indices[i], &lu[i], &bd[i], nodeest[i], cbhandle, &random);    
    }

    return status;
};

int solve_c(int size, int** matrix) {
    int num_vars = size * size;
    
    RANKING r = {.size = size, .num_vars = num_vars, .matrix = matrix};
    CUT_ARRAY allcuts;
    generateCuts(&allcuts, r);
    USER_HANDLE u = {.r = r, .allcuts = allcuts};

    int status = 0;
    char errbuf[CPXMESSAGEBUFSIZE];

    CPXENVptr env = NULL;
    CPXLPptr lp = NULL;


    double lb[num_vars], ub[num_vars], obj[num_vars];
    char ctype[num_vars];
    char cnamebuf[num_vars][32];
    char *cname[num_vars];

    int i, j, ind;

    int ncuts;
    double tol, objval;
    double x[num_vars];

    int fromtable = 0;
    int lazy = 0;
    int usecallback = 1;

    /* Create CPLEX environment and model. */
    env = CPXXopenCPLEX(&status);
    /* Set the env ptr in the user handle */
    u.envptr = env;
    if ( status != 0 ) {
        fprintf(stderr, "Failed to open CPLEX: %s\n", CPXXgeterrorstring(env, status, errbuf));
        goto TERMINATE;
    }

    status = CPXXsetintparam(env, CPXPARAM_ScreenOutput, CPX_ON);
    if ( status != 0 ) {
        fprintf(stderr, "Failed to enable screen output: %s\n", CPXXgeterrorstring(env, status, errbuf));
        goto TERMINATE;
    }

    lp = CPXXcreateprob(env, &status, "admipex5");
    if ( status != 0 ) {
        fprintf(stderr, "Failed to create problem: %s\n", CPXXgeterrorstring(env, status, errbuf));
        goto TERMINATE;
    }

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            ind = i * size + j;
            lb[ind] = 0.0;
            ub[ind] = 1.0;
            ctype[ind] = 'B';
            obj[ind] = matrix[i][j];
            sprintf(cnamebuf[ind], "x_%d_%d", i, j);
            cname[ind] = cnamebuf[ind];
        }
    }

    status = CPXXnewcols(env, lp, num_vars, obj, lb, ub, ctype, cname);
    if (status != 0) {
        fprintf(stderr, "Failed to create variables: %s\n", CPXXgeterrorstring(env, status, errbuf));
        goto TERMINATE;  
    }

    status = addPrecedenceConstraints(env, lp, size, matrix);
    if ( status != 0 ) {
        fprintf(stderr, "Failed to add precedence constraints: %s\n", CPXXgeterrorstring(env, status, errbuf));
        goto TERMINATE;
    }

    if ( !lazy ) {
        status = addDicycleConstraints(env, lp, size, matrix);
        if ( status != 0 ) {
            fprintf(stderr, "Failed to add dicycle constraints: %s\n", CPXXgeterrorstring(env, status, errbuf));
            goto TERMINATE;
        }
    }

    if ( (status = CPXXsetintparam(env, CPXPARAM_Threads, 1)) ||
        (status = CPXXsetintparam(env, CPXPARAM_MIP_Strategy_HeuristicFreq, -1)) ||
        (status = CPXXsetintparam(env, CPXPARAM_MIP_Cuts_MIRCut, -1)) ||
        (status = CPXXsetintparam(env, CPXPARAM_MIP_Cuts_Implied, -1)) ||
        (status = CPXXsetintparam(env, CPXPARAM_MIP_Cuts_Gomory, -1)) ||
        (status = CPXXsetintparam(env, CPXPARAM_MIP_Cuts_FlowCovers, -1)) ||
        (status = CPXXsetintparam(env, CPXPARAM_MIP_Cuts_PathCut, -1)) ||
        (status = CPXXsetintparam(env, CPXPARAM_MIP_Cuts_LiftProj, -1)) ||
        (status = CPXXsetintparam(env, CPXPARAM_MIP_Cuts_ZeroHalfCut, -1)) ||
        (status = CPXXsetintparam(env, CPXPARAM_MIP_Cuts_Cliques, -1)) ||
        (status = CPXXsetintparam(env, CPXPARAM_MIP_Cuts_Covers, -1)) ) {
        fprintf(stderr, "Failed to set parameter: %s\n",
                CPXXgeterrorstring(env, status, errbuf));
        goto TERMINATE;
    }

    /* Setup callbacks. We disable CPXPARAM_MIP_Strategy_CallbackReducedLP so
    * that indices in the callbacks can refer to the original model and we
    * don't have to translate the indices to the presolved model.
    * This also requires us to disable non-linear reductions so that cuts and
    * lazy constraints can always be crushed.
    */
    status = CPXXsetintparam(env, CPXPARAM_MIP_Strategy_CallbackReducedLP, CPX_OFF);
    if ( status != 0 ) {
        fprintf(stderr, "Failed to disable reduced LP in callbacks: %s\n", CPXXgeterrorstring(env, status, errbuf));
        goto TERMINATE;
    }
    status = CPXXsetintparam(env, CPXPARAM_Preprocessing_Linear, 0);
    if ( status != 0 ) {
        fprintf(stderr, "Failed to disable dual reductions: %s\n", CPXXgeterrorstring(env, status, errbuf));
        goto TERMINATE;
    }
    status = CPXXsetintparam(env, CPX_PARAM_PREIND, 0);
    if ( status != 0 ) {
        fprintf(stderr, "Failed to disable presolve: %s\n", CPXXgeterrorstring(env, status, errbuf));
        goto TERMINATE;
    }
    status = CPXXsetintparam(env, CPXPARAM_Preprocessing_Dual, -1);
    if ( status != 0 ) {
        fprintf(stderr, "Failed to disable dual presolve: %s\n", CPXXgeterrorstring(env, status, errbuf));
        goto TERMINATE;
    }
    status = CPXXsetintparam(env, CPX_PARAM_MIPINTERVAL, 1);
    status = CPXXsetintparam(env, CPXPARAM_MIP_Limits_Nodes, 1);

    if ( lazy ) {
        // int nCols = CPXXgetnumcols(env, lp);
        // int nRows = CPXXgetnumrows(env, lp);
        // printf("Number of cols at start: %d\n", nCols);
        // printf("Number of rows at start: %d\n", nRows);
        // status = CPXXsetlazyconstraintcallbackfunc(env, lazycallback, &u);
        // status = CPXXsetusercutcallbackfunc(env, cutcallback, &u);
        if ( status != 0 ) {
            fprintf(stderr, "Failed to add callback: %s\n", CPXXgeterrorstring(env, status, errbuf));
            goto TERMINATE;
        }
    }
    status = CPXXsetbranchcallbackfunc(env, branchcallback, &u);

    /* Solve the model. */
    CPXXchgobjsen (env, lp, CPX_MAX);
    status = CPXXmipopt(env, lp);
    if ( status != 0 ) {
        fprintf(stderr, "Failed to optimize: %s\n", CPXXgeterrorstring(env, status, errbuf));
        goto TERMINATE;
    }

    /* Query solution and some statistics that we want to display. */
    status = CPXXgetnumcuts(env, lp, CPX_CUT_USER, &ncuts);
    if ( status != 0 ) {
        fprintf(stderr, "Failed to query cut counts: %s\n", CPXXgeterrorstring(env, status, errbuf));
        goto TERMINATE;
    }

    status = CPXXgetobjval(env, lp, &objval);
    if ( status != 0 ) {
        fprintf(stderr, "Failed to query objective: %s\n", CPXXgeterrorstring(env, status, errbuf));
        goto TERMINATE;
    }

    status = CPXXgetx(env, lp, x, 0, num_vars - 1);
    if ( status != 0 ) {
        fprintf(stderr, "Failed to query solution vector: %s\n", CPXXgeterrorstring(env, status, errbuf));
        goto TERMINATE;
    }

    status = CPXXgetdblparam(env, CPXPARAM_MIP_Tolerances_Integrality, &tol);
    if ( status != 0 ) {
        fprintf(stderr, "Failed to query tolerance: %s\n", CPXXgeterrorstring(env, status, errbuf));
        goto TERMINATE;
    }

    /* Dump the solution. */
    printf("Solution status:                   %d\n", CPXXgetstat(env, lp));
    printf("Nodes processed:                   %d\n", CPXXgetnodecnt(env, lp));
    printf("Active user cuts/lazy constraints: %d\n", ncuts);
    printf("Optimal value:                     %f\n", objval);
    
    TERMINATE:

    /* Free the problem as allocated by CPXcreateprob and CPXreadcopyprob, if necessary */
    if ( lp != NULL ) {
        int xstatus = CPXXfreeprob (env, &lp);

        if ( !status ) status = xstatus;
    }

    /* Free the CPLEX environment, if necessary */
    if ( env != NULL ) {
        int xstatus = CPXXcloseCPLEX (&env);

        if ( !status ) status = xstatus;
    }
     
    return (status);
}
