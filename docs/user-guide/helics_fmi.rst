==================
HELICS FMI
==================

```
HELICS-FMI for loading and executing FMU's with HELICS
Usage: helics-fmi [OPTIONS]

Options:
  -v,--version                specify the versions of helics and helics-fmi
  --integrator TEXT:{cvode,arkode,boost} [cvode]
                              the type of integrator to use(cvode, arkode, boost)
  --integrator-args TEXT      arguments to pass to the integrator
  --step FLOAT                the step size to use (specified in seconds or as a time string (10ms)
  --stop FLOAT                the time to stop the simulation (specified in seconds or as a time string (10ms)
  --brokerargs TEXT           arguments to pass to an automatically generated broker
  -h,-?,--help                print this help module
  --config-file               Read an ini file
  --output_variables TEXT ... Specify outputs of the FMU by name
  --input_variables TEXT ...  Specify the input variables of the FMU by name
  --connections TEXT ...      Specify connections this FMU should make
  --cosim                     specify that the fmu should run as a co-sim FMU if possible
  --modelexchange{false}      specify that the fmu should run as a model exchange FMU if possible
[Option Group: input files]
   REQUIRED
  Positionals:
    inputfile TEXT:FILE         specify the input files
  Options:
    -i,--input TEXT:FILE ...    specify the input files
```