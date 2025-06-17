# Search Schemes Online tool
Website: https://sgssgene.github.io/web_search_schemes/

## How to build

```
mkdir build-emcc; cd $_
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
cp search_schemes.js.{js,wasm} ../docs
```
