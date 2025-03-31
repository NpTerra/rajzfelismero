#pragma once

#include "vector.h"
#include "canvas.h"


typedef struct Node {
    double value, bias, output;
    /** Pointer to activation function. */
    double (*act)(double);
    /** Connection weights to each Node in the next layer. */
    Vector con;
} Node;


typedef struct MLP {
    size_t x, y, kx, ky;
    char *name;
    Vector layers;
    Canvas canvas;
    Canvas draw_canvas;
    size_t result;
} MLP;


/**
 * Creates a new MLP model with a given name and number of layers.
 * The returned MLP should be freed by the caller,
 * as it contains dynamically allocated memory.
 * 
 * \param x Canvas width.
 * \param y Canvas Height.
 * \param kx MaxPool2D kernel width.
 * \param ky MaxPool2D kernel height.
 * \param name String with the name to copy.
 * \param layers Number of layers to start with.
 * 
 * \returns The newly created MLP struct.
 */
MLP create_mlp(size_t x, size_t y, size_t kx, size_t ky, const char *name, size_t layers);


/**
 * Inserts a new layer at the end of an MLP with a given number of dummy nodes.
 * The new layer will be automatically connected to the layer before it with weights of 1.0 and biases of 0.0.
 * 
 * \param mlp Pointer to the target MLP.
 * \param nodes Number of dummy nodes.
 */
void add_mlp_layer(MLP *mlp, size_t nodes);


/**
 * Sets the activation function of all nodes in a layer to ReLU.
 * ReLU will set each negative value to zero and keeps the others.
 * 
 * \param mlp Pointer to the target MLP.
 * \param layer The layer's index inside the MLP.
 */
void set_layer_relu(MLP *mlp, size_t layer);


/**
 * Sets the activation function of all nodes in a layer to linear.
 * The linear function will keep each value as is.
 * 
 * \param mlp Pointer to the target MLP.
 * \param layer The layer's index inside the MLP.
 */
void set_layer_linear(MLP *mlp, size_t layer);


/**
 * Inserts a new Node at the end of a layer in an MLP.
 * The new Node will have default connections with 1.0 as weight.
 * 
 * \param mlp Pointer to the target MLP.
 * \param layer The layer's index inside the MLP.
 * \param bias The new Node's bias.
 */
void push_mlp(MLP *mlp, size_t layer, double bias);


/**
 * Sets a Node's bias in an MLP.
 * 
 * \param mlp Pointer to the target MLP.
 * \param layer The layer's index inside the MLP.
 * \param n The Node's index insite the layer.
 * \param bias The Node's new bias. 
 */
void set_node_bias(MLP *mlp, size_t layer, size_t n, double bias);


/**
 * Frees all the dynamically allocated memory used by the model.
 * 
 * \param mlp Pointer to the MLP.
 */
void free_mlp(MLP *mlp);


/**
 * Loads the contents of an MLP's Canvas to the input layer.
 * The MaxPooling is also done by this step.
 * 
 * \param mlp Pointer to the target MLP.
 */
void load_mlp_input(MLP *mlp);


/**
 * Runs the MLP model in a simple feed-forward manner,
 * calculating the output for the current input layer.
 * 
 * \param mlp Pointer to the target MLP.
 */
void run_mlp(MLP *mlp);
