#include "debugmalloc.h"
#include "filehandler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "errors.h"
#include "snippets.h"


/**
 * Reads and processes a given amount of instructions from a file.
 * 
 * \param f The file to read from.
 * \param mlp Pointer to the target MLP.
 * \param n Number of instructions.
 * 
 * \returns An RSTATUS with the possible status codes.
 */
static RSTATUS read_instructions(FILE *f, MLP *mlp, int n)
{
    char buf[50+1];
    size_t l;
    for(size_t i = 0; i < n; i++)
    {
        if(fscanf(f, "%50s", buf) != 1)
            return NODATA;

        if(strcmp(buf, "layer") == 0)
        {
            if(fscanf(f, "%zu", &l) != 1)
                return NODATA;
            
            add_mlp_layer(mlp, l);

            continue;
        }

        if(strcmp(buf, "relu") == 0)
        {
            set_layer_relu(mlp, mlp->layers.size-1);

            continue;
        }

        return WRONGINSTRUCTION;
    }

    return SUCCESS;
}


/**
 * Reads and processes the biases of each Node in an MLP from a file.
 * 
 * \param f The file to read from.
 * \param mlp Pointer to the target MLP.
 * 
 * \returns An RSTATUS with the possible status codes.
 */
static RSTATUS read_biases(FILE *f, MLP *mlp)
{
    for(size_t i = 1; i < mlp->layers.size; i++)
    {
        Vector *layer = &get_vector_as_type(&mlp->layers, i, Vector);
        for(size_t j = 0; j < layer->size; j++)
        {
            double bias;
            if(fscanf(f, "%lf", &bias) != 1)
                return NODATA;
            
            set_node_bias(mlp, i, j, bias);
        }
    }

    return SUCCESS;
}


/**
 * Reads and processes the necessary weights for each Node in an MLP from a file.
 * 
 * \param f The file to read from.
 * \param mlp Pointer to the target MLP.
 * 
 * \returns An RSTATUS with the possible status codes.
 */
static RSTATUS read_weights(FILE *f, MLP *mlp)
{
    for(size_t i = 1; i < mlp->layers.size; i++)
    {
        Vector *curr = &get_vector_as_type(&mlp->layers, i, Vector);
        Vector *prev = curr-1;
        
        for(size_t j = 0; j < curr->size; j++)
        {
            for(size_t k = 0; k < prev->size; k++)
            {
                Node *node = &get_vector_as_type(prev, k, Node);
                double *d = &get_vector_as_type(&node->con, j, double);

                if(fscanf(f, "%lf", d) != 1)
                    return NODATA;
            }
        }
    }

    return SUCCESS;
}


ReadResult read_model(const char *path, const char *name)
{
    #define pass(x, y, f, s) if((x) != (y)) {fclose(f); return (ReadResult){(s), {0}};}
    
    FILE *f = fopen(path, "r");

    if(f == NULL)
        return (ReadResult){NOFILE, {0}};

    // canvas size
    size_t x, y;
    pass(fscanf(f, "%zux%zu", &x, &y), 2, f, NODATA);

    // MaxPool2D kernel size
    size_t kx, ky;
    pass(fscanf(f, "%zux%zu", &kx, &ky), 2, f, NODATA);

    pass(kx > 0 && ky > 0 && x % kx == 0 && y % ky == 0, true, f, KERNELSIZE);

    MLP mlp = create_mlp(x, y, kx, ky, name, 1);

    #undef pass
    #define pass(x, y, f, s) if((x) != (y)) {fclose(f); free_mlp(&mlp); return (ReadResult){(s), {0}};}

    // add the input layer
    add_mlp_layer(&mlp, (x/kx)*(y/ky));
    // number of instructions
    int n;
    pass(fscanf(f, "%d", &n), 1, f, NODATA);

    // instructions
    RSTATUS inst = read_instructions(f, &mlp, n);
    pass(inst, SUCCESS, f, inst);

    pass(mlp.layers.size >= 2, true, f, NOLAYER);

    // biases
    RSTATUS bias = read_biases(f, &mlp);
    pass(bias, SUCCESS, f, bias);

    // weights
    RSTATUS weight = read_weights(f, &mlp);
    pass(weight, SUCCESS, f, weight);

    fclose(f);

    return (ReadResult){SUCCESS, mlp};

    #undef pass
}


bool write_model_result(MLP *mlp, WRITEMODE mode)
{
    FILE *file = NULL;
    char *fname = NULL;
    if(mode == DISK || mode == ALL)
    {
        fname = (char*) malloc(strlen(mlp->name)+5);
        strcpy(fname, mlp->name);
        strcat(fname, ".txt");

        file = fopen((const char*) fname, "w");
        if(file == NULL)
            return false;
    }

    Vector *v = &get_vector_as_type(&mlp->layers, mlp->layers.size-1, Vector);
    Node *n = &get_vector_as_type(v, 0, Node);

    size_t mind = 0;
    double ma = n->output;
    for(size_t i = 0; i < v->size; i++)
    {
        n = &get_vector_as_type(v, i, Node);
        if(mode == DISK || mode == ALL)
            fprintf(file, "%zu: %.2lf%% ", i, n->output*100);
        if(mode == CONSOLE || mode == ALL)
            printf("%zu: %.2lf%% ", i, n->output*100);
        
        if(n->output >= ma) {
            ma = n->output; mind = i;
        }
    }

    mlp->result = mind;
    
    if(mode == DISK || mode == ALL)
    {
        fprintf(file, "\nScore: %zu\n", mind);
        free(fname);
        fclose(file);
    }
    if(mode == CONSOLE || mode == ALL)
        printf("\nScore: %zu\n", mind);

    return true;
}
