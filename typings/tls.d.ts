import { TLSSocket } from "tls";

declare namespace tlsInternal {

  interface TlsWrapOptions {
    ca?: string
    rejectUnauthorized?: boolean
    servername?: string
  }

  class TlsWrap {
    private jsref?: TLSSocket
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
