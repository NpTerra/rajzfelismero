#include "debugmalloc.h"
#include "mlp.h"
#include <stdlib.h>
#include <math.h>

#include "errors.h"
#include "snippets.h"



static double relu(double value)
{
    return max(0, value);
}


static double linear(double value)
{
    return value;
}


/**
 * Creates a new Node with a given bias.
 * The new Node should be freed by the caller
 * as it contains dynamically allocated memory.
 * 
 * \param bias The Node's bias.
 * 
 * \returns The new Node.
 */
static Node create_node(double bias)
{
    return (Node){0, bias, 0,
        linear,
        create_vector(1, sizeof(double), false)
    };
}


MLP create_mlp(size_t x, size_t y, size_t kx, size_t ky, const char *name, size_t layers)
{
    MLP m;
    m.x = x;
    m.y = y;
    m.kx = kx;
    m.ky = ky;
    m.name = strclone(name);
    m.layers = create_vector(layers, sizeof(Vector), false);
    m.canvas = create_canvas(x, y);
    m.draw_canvas = create_canvas(x, y);
    m.result = 0;

    return m;
}


void add_mlp_layer(MLP *mlp, size_t nodes)
{
    Vector layer = create_vector(nodes, sizeof(Node), false);
    push_vector(&mlp->layers, &layer);

    for(size_t i = 0; i < nodes; i++)
        push_mlp(mlp, mlp->layers.size-1, 0.0);
}


void set_layer_relu(MLP *mlp, size_t layer)
{
    Vector l = get_vector_as_type(&mlp->layers, layer, Vector);

    for(size_t i = 0; i < l.size; i++)
    {
        Node *n = &get_vector_as_type(&l, i, Node);
        n->act = relu;
    }
}


void set_layer_linear(MLP *mlp, size_t layer)
{
    Vector l = get_vector_as_type(&mlp->layers, layer, Vector);

    for(size_t i = 0; i < l.size; i++)
    {
        Node *n = &get_vector_as_type(&l, i, Node);
        n->act = linear;
    }
}


void push_mlp(MLP *mlp, size_t layer, double bias)
{
    double def = 1.0;

    Node node = create_node(bias);
    Vector *curr = &get_vector_as_type(&mlp->layers, layer, Vector);

    if(layer > 0)
    {
        Vector *prev = &get_vector_as_type(&mlp->layers, layer-1, Vector);
        for(size_t i = 0; i < prev->size; i++)
        {
            Node *pn = &get_vector_as_type(prev, i, Node);
            push_vector(&pn->con, &def);
        }
    }

    if(layer < mlp->layers.size-1)
    {
        Vector *next = &get_vector_as_type(&mlp->layers, layer, Vector);
        for(size_t i = 0; i < next->size; i++)
        {
            push_vector(&node.con, &def);
        }
    }

    push_vector(curr, &node);
}


void set_node_bias(MLP *mlp, size_t layer, size_t n, double bias)
{
    Vector *l = &get_vector_as_type(&mlp->layers, layer, Vector);
    Node *node = &get_vector_as_type(l, n, Node);
    node->bias = bias;
}


void free_mlp(MLP *mlp)
{   
    if(mlp == NULL || mlp->name == NULL) return;

    free(mlp->name);

    Vector *layer;
    Node *node;
    for(size_t i = 0; i < mlp->layers.size; i++)
    {
        layer = &get_vector_as_type(&mlp->layers, i, Vector);
        for(size_t j = 0; j < layer->size; j++)
        {
            node = &get_vector_as_type(layer, j, Node);
            free_vector(&node->con);
        }
        free_vector(layer);
    }

    free_vector(&mlp->layers);
    free_canvas(&mlp->canvas);
    free_canvas(&mlp->draw_canvas);

    mlp->name = NULL;
}


/**
 * Finds the maximum value inside a given 2D area of an MLP's Canvas.
 * 
 * \param mlp Pointer to the target MLP.
 * \param x X coordinate of the area's top left corner.
 * \param y Y coordinate of the area's top left corner.
 * \param width The area's width.
 * \param height The area's height.
 * 
 * \returns The maximum value inside the given area.
 */
static double maxpool2d(MLP *mlp, size_t x, size_t y, size_t width, size_t height)
{
    double m = get_canvas_xy(&mlp->draw_canvas, x, y)/255.0;
    for(size_t i = x; i < x+width; i++)
    {
        for(size_t j = y; j < y+height; j++)
        {
            m = max(m, get_canvas_xy(&mlp->canvas, i, j)/255.0);
        }
    }

    return m;
}


void load_mlp_input(MLP *mlp)
{
    size_t n1 = mlp->x/mlp->kx;
    size_t n2 = mlp->y/mlp->ky;

    Vector *vx = &get_vector_as_type(&mlp->layers, 0, Vector);

    for(size_t x = 0; x < n1; x++)
    {
        for(size_t y = 0; y < n2; y++)
        {
            Node *vy = &get_vector_as_type(vx, y*n1 + x, Node);
            vy->value = maxpool2d(mlp, x*mlp->kx, y*mlp->ky, mlp->kx, mlp->ky);
        }
    }
}


/**
 * Calculates and sets a given Node's output
 * based on its value, bias and activation function.
 * 
 * \param node Pointer to the target Node.
 */
static void act_node(Node *node)
{
    node->output = node->act(node->value + node->bias);
}


/**
 * Calculates and sets a given Node's value
 * and output. The output is based on the
 * Node's value, bias and activation function.
 * 
 * The target Node's value will be the sum of
 * each previous Node's output multiplied by the
 * weight of its connection to the target Node.
 * 
 * \param layer Pointer to the previous layer inside an MLP.
 * \param node Pointer to the target Node.
 */
static void run_node(Vector *layer, Node *node, size_t node_index)
{
    node->value = 0;
    for(size_t i = 0; i < layer->size; i++)
    {
        Node *p = &get_vector_as_type(layer, i, Node);
        node->value += p->output * get_vector_as_type(&p->con, node_index, double);
    }
    act_node(node);
}


/**
 * Applies softmax to a Vector of Nodes.
 * 
 * Each Node's output will be overridden by the probability associated with its current output.
 * 
 * \param layer Pointer to the target Vector.
 */
static void softmax(Vector *layer)
{
    double sum = 0;
    for(size_t i = 0; i < layer->size; i++)
    {
        Node *n = &get_vector_as_type(layer, i, Node);
        n->output = exp(n->output);
        sum += n->output;
    }

    for(size_t i = 0; i < layer->size; i++)
    {
        Node *n = &get_vector_as_type(layer, i, Node);
        n->output = n->output/sum;
    }
}


void run_mlp(MLP *mlp)
{
    Vector *input = &get_vector_as_type(&mlp->layers, 0, Vector);
    Vector *output = &get_vector_as_type(&mlp->layers, mlp->layers.size-1, Vector);
    for(size_t i = 0; i < input->size; i++)
    {
        Node *node = &get_vector_as_type(input, i, Node);
        act_node(node);
    }

    for(size_t i = 1; i < mlp->layers.size; i++)
    {
        Vector *prev = &get_vector_as_type(&mlp->layers, i-1, Vector);
        Vector *curr = &get_vector_as_type(&mlp->layers, i, Vector);
        for(size_t j = 0; j < curr->size; j++)
        {
            Node *node = &get_vector_as_type(curr, j, Node);
            run_node(prev, node, j);
        }
    }

    softmax(output);
}
