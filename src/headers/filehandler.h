#pragma once

#include "mlp.h"


/** Status codes for the file reading. */
typedef enum RSTATUS {
    SUCCESS = 0,        /*!< The code finished successfully. */
    NOFILE,             /*!< The file couldn't be opened. */
    NODATA,             /*!< Couldn't read the requested data from a file or EOF is reached. */
    KERNELSIZE,         /*!< The kernel size is invalid for the MaxPool2D operation. */
    NOLAYER,            /*!< There isn't enough layers in the MLP. */
    WRONGINSTRUCTION    /*!< The given instruction doesn't exist. */
} RSTATUS;


/** Contains a status code and an MLP if the reading was successful. */
typedef struct ReadResult {
    /** The status code of the reading. Can be either SUCCESS or an error code.*/
    RSTATUS status;
    /** The returned model if the reading was successfuly. Empty otherwise. */
    MLP model;
} ReadResult;

/** Writing mode selector. */
typedef enum WRITEMODE {
    DISK,       /*!< The writing should be to a file on the disk. */
    CONSOLE,    /*!< The writing should be directed to the standard output. */
    ALL         /*!< The writing should target both a file and the stdout. */
} WRITEMODE;


/**
 * Tries to read a model from a file at the given path.
 * The function assumes that the file ends with the .mlpmodel extension.
 * 
 * \param path The path to the file.
 * \param name The file's name without extension.
 * 
 * \returns A ReadResult struct, containing the read's status code
 * and a valid MLP struct if the status code is SUCCESS.
 * If the MLP is valid, then it needs to be freed later by the caller. 
 */
ReadResult read_model(const char *path, const char *name);


/**
 * Writes the current output probabilities of an MLP either into a file, to the standard output or both.
 * 
 * \param mlp Pointer to the target MLP.
 * \param mode Specifies where the function should write the MLP's output.
 * 
 * \returns True if the writing was successful.
 */
bool write_model_result(MLP *mlp, WRITEMODE mode);
