from __future__ import print_function, absolute_import, division  # makes these scripts backward compatible with python 2.6 and 2.7

available_convergence_accelerators = {
    "constant" : "constant_convergence_accelerator",
    "aitken"   : "aitken",
    "iqnils"   : "iqnils",
    "mvqn"     : "mvqn",
}

def CreateConvergenceAccelerator(settings, solvers, cosim_solver_details, level):
    """This function creates and returns the convergence accelerator used for CoSimulation
    New convergence accelerators have to be registered by adding them to "available_convergence_accelerators"
    """
    if (type(settings) != dict):
        raise Exception("Input is expected to be provided as a python dictionary")

    accelerator_type = settings["type"]

    if accelerator_type in available_convergence_accelerators:
        accelerator_module = __import__(available_convergence_accelerators[accelerator_type])
        return accelerator_module.Create(settings, solvers, cosim_solver_details, level)
    else:
        err_msg  = 'The requested convergence accelerator "' + accelerator_type + '" is not available!\n'
        err_msg += 'The following convergence accelerators are available:\n'
        for avail_accelerator in available_convergence_accelerators:
            err_msg += "\t" + avail_accelerator + "\n"
        raise NameError(err_msg)
