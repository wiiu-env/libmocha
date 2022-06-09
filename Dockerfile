FROM wiiuenv/devkitppc:20220605

RUN git clone --depth 1 --single-branch -b filesystemstructs https://github.com/Maschell/wut && cd wut && git reset --hard a060c981e23cdd7e622624692df2d789693432ce && make install && cd .. && rm -rf wut

WORKDIR tmp_build
COPY . .
RUN make clean && make && mkdir -p /artifacts/wut/usr && cp -r lib /artifacts/wut/usr && cp -r include /artifacts/wut/usr
WORKDIR /artifacts

FROM scratch
COPY --from=0 /artifacts /artifacts