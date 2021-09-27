FROM frolvlad/alpine-miniconda3

MAINTAINER Hans Erik Thrane <info@roq-trading.com>

COPY pkgs/ /pkgs/

RUN conda install --quiet --yes --override-channels --channel file:///pkgs --channel defaults roq-kucoin-futures
RUN rm -rf /pkgs

ENTRYPOINT [ "/opt/conda/bin/roq-kucoin-futures" ]

CMD [ "--help" ]
