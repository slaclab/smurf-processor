FROM tidair/smurf-rogue:R1.1.0

# Install the SMURF PCIe card repository
WORKDIR /usr/local/src
RUN git clone https://github.com/slaclab/smurf-pcie.git
WORKDIR smurf-pcie
RUN sed -i -e 's|git@github.com:|https://github.com/|g' .gitmodules
RUN git submodule sync && git submodule update --init --recursive
ENV PYTHONPATH /usr/local/src/smurf-pcie/software/python:${PYTHONPATH}
ENV PYTHONPATH /usr/local/src/smurf-pcie/firmware/submodules/axi-pcie-core/python:${PYTHONPATH}

# Install smurf-processor
WORKDIR /usr/local/src
RUN git clone https://github.com/slaclab/smurf-processor.git -b ESCRYODET-402
WORKDIR smurf-processor
RUN mkdir build
WORKDIR build
RUN cmake -DCMAKE_BUILD_TYPE=Release .. && make
ENV PYTHONPATH /usr/local/src/smurf-processor/lib:${PYTHONPATH}
ENV PYTHONPATH /usr/local/src/smurf-processor/python:${PYTHONPATH}
ENV SMURF_DIR /usr/local/src/smurf-processor

# Add utilities
RUN mkdir -p /usr/local/src/smurf-processor_utilities
ADD scripts/* /usr/local/src/smurf-processor_utilities
ENV PATH /usr/local/src/smurf-processor_utilities:${PATH}