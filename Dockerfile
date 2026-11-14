# Server image: builds myteams_server and myteams_cli, runs the server on port 8080.
FROM debian:bookworm-slim AS build
RUN apt-get update && apt-get install -y --no-install-recommends clang make uuid-dev \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY Makefile ./
COPY include include
COPY src src
COPY libs libs
RUN make all

FROM debian:bookworm-slim
RUN apt-get update && apt-get install -y --no-install-recommends libuuid1 \
    && rm -rf /var/lib/apt/lists/*
COPY --from=build /src/myteams_server /src/myteams_cli /usr/local/bin/
# State is saved to myteams.dat in the working directory on SIGINT
WORKDIR /data
EXPOSE 8080
STOPSIGNAL SIGINT
CMD ["myteams_server", "8080"]
