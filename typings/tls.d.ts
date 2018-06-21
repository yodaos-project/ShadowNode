declare namespace tlsInternal {
  import { TLSSocket } from "tls";

  interface TlsWrapOptions {
    ca?: string
    rejectUnauthorized?: boolean
    servername?: string
  }

  class TlsWrap {
    jsref?: TLSSocket
    onread?: void
    onwrite?: void
    onclose?: void
    onhandshakedone?: () => void

    constructor (options?: TlsWrapOptions)
    end (): void
    handshake (): void
    write (chunk: Buffer): Buffer
    read (chunk: Buffer): number
  }
}
