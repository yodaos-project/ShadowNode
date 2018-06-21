declare module "tcp" {
  import { Socket } from 'net'

  class Handle {
    write (chunk: Buffer, callback: (status: number) => void): void
    connect (ip: string, port: number, callback: (status: number) => void): void
    close (): void
    readStart (): void

    setKeepAlive?: (enable: boolean, delay: number) => void
    getpeername?: (inout: object) => number
    getsockname?: (inout: object) => number

    owner?: Socket
    onclose?: () => void
    onread?: (socket: Socket, nread: number, isEOF: boolean, buffer: Buffer) => void

    // Server Socket
    shutdown (callback: () => void): void
    bind (host: string, port: number): number
    listen (backlog: number): void
    onconnection?: (status: number, clientHandle: Handle) => void
  }

  class TcpHandle extends Handle {}

  export = TcpHandle
}
