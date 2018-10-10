var emitter = new EventEmitter();
var listenr = () => {
  emitter.removeListener('test', listener);
};
emitter.on('test', listener);
