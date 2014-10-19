var fs = require('fs')
var stream = require('stream');

/* necessário instalar este pacote via NPM na pasta deste arquivo 
   npm install iconv-lite; */
   
var iconv = require('iconv-lite');


/* busca a pasta de instalação do Houaiss
(mude se estiver instalado em outro lugar) */
var pastaHouaiss = "C:/Program Files (x86)/Houaiss3/dicionario/";
var deah = fs.readdirSync(pastaHouaiss);

/* em cada arquivo do dicionário executa a desofuscação */
for (var i = 0; i < deah.length; i++) {

    /* lê o arquivo do dicionário */
	var input = fs.createReadStream(pastaHouaiss + deah[i], 'utf-8');

	/* prepara a saída do arquivo desofuscado */
	var output = fs.createWriteStream(deah[i] + '.txt', 'utf-8');

    /* onde mágica acontece */
    /* cada "chunk" é um byte */
	var decode = new stream.Transform;
	decode._transform = function(chunk, encoding, done) {

		for (var i = 0; i < chunk.length; i++)
			chunk[i] = chunk[i] + 0x0B;

		done(null, chunk);
	};

    /* onde todas as ações são invocadas */
	input
		.pipe(decode)
		.pipe(iconv.decodeStream('iso-8859-1')) /* lê a versão original do arquivo que está em iso */
		.pipe(iconv.encodeStream('utf-8'))      /* e muda para o formato utf-8 que todos nós amamos */
		.pipe(output);
}
