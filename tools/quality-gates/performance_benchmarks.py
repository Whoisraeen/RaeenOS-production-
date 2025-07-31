#!/usr/bin/env python3
"""
RaeenOS Performance Benchmarking Suite
Comprehensive performance testing for 42-agent development
Version: 1.0
Author: Testing & QA Automation Lead
"""

import json
import time
import subprocess
import statistics
import sys
import argparse
import os
from pathlib import Path
from typing import Dict, List, Any, Optional
from dataclasses import dataclass, asdict
import threading
import psutil
import numpy as np

@dataclass
class PerformanceMetric:
    """Represents a single performance metric"""
    name: str
    value: float
    unit: str
    target: float
    status: str  # "pass", "fail", "warning"
    timestamp: float
    
    def to_dict(self) -> Dict[str, Any]:
        return asdict(self)

@dataclass
class BenchmarkResult:
    """Represents the result of a benchmark suite"""
    agent_name: str
    benchmark_type: str
    timestamp: float
    duration: float
    metrics: List[PerformanceMetric]
    overall_status: str
    system_info: Dict[str, Any]
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "agent_name": self.agent_name,
            "benchmark_type": self.benchmark_type,
            "timestamp": self.timestamp,
            "duration": self.duration,
            "metrics": [m.to_dict() for m in self.metrics],
            "overall_status": self.overall_status,
            "system_info": self.system_info
        }

class SystemMonitor:
    """Monitor system resources during benchmarks"""
    
    def __init__(self, sampling_interval=0.1):
        self.sampling_interval = sampling_interval
        self.monitoring = False
        self.cpu_samples = []
        self.memory_samples = []
        self.io_samples = []
        self.thread = None
    
    def start(self):
        """Start monitoring system resources"""
        self.monitoring = True
        self.cpu_samples = []
        self.memory_samples = []
        self.io_samples = []
        self.thread = threading.Thread(target=self._monitor)
        self.thread.start()
    
    def stop(self) -> Dict[str, Any]:
        """Stop monitoring and return statistics"""
        self.monitoring = False
        if self.thread:
            self.thread.join()
        
        return {
            "cpu": {
                "avg": statistics.mean(self.cpu_samples) if self.cpu_samples else 0,
                "max": max(self.cpu_samples) if self.cpu_samples else 0,
                "min": min(self.cpu_samples) if self.cpu_samples else 0,
                "std": statistics.stdev(self.cpu_samples) if len(self.cpu_samples) > 1 else 0
            },
            "memory": {
                "avg": statistics.mean(self.memory_samples) if self.memory_samples else 0,
                "max": max(self.memory_samples) if self.memory_samples else 0,
                "min": min(self.memory_samples) if self.memory_samples else 0,
                "std": statistics.stdev(self.memory_samples) if len(self.memory_samples) > 1 else 0
            }
        }
    
    def _monitor(self):
        """Internal monitoring loop"""
        while self.monitoring:
            try:
                cpu_percent = psutil.cpu_percent(interval=None)
                memory_percent = psutil.virtual_memory().percent
                
                self.cpu_samples.append(cpu_percent)
                self.memory_samples.append(memory_percent)
                
                time.sleep(self.sampling_interval)
            except Exception:
                continue

class PerformanceBenchmark:
    """Base class for performance benchmarks"""
    
    def __init__(self, agent_name: str, component_path: str):
        self.agent_name = agent_name
        self.component_path = Path(component_path)
        self.monitor = SystemMonitor()
        
        # Get system information
        self.system_info = {
            "cpu_count": psutil.cpu_count(),
            "cpu_freq": psutil.cpu_freq()._asdict() if psutil.cpu_freq() else {},
            "memory_total": psutil.virtual_memory().total,
            "platform": sys.platform,
            "python_version": sys.version
        }
    
    def run_benchmark(self, benchmark_type: str) -> BenchmarkResult:
        """Run a specific benchmark type"""
        print(f"Running {benchmark_type} benchmark for {self.agent_name}...")
        
        start_time = time.time()
        self.monitor.start()
        
        try:
            metrics = self._execute_benchmark(benchmark_type)
        except Exception as e:
            print(f"Error running benchmark: {e}")
            metrics = [PerformanceMetric(
                name="benchmark_error",
                value=0,
                unit="error",
                target=0,
                status="fail",
                timestamp=time.time()
            )]
        finally:
            resource_stats = self.monitor.stop()
            end_time = time.time()
        
        # Add resource utilization metrics
        metrics.extend([
            PerformanceMetric(
                name="cpu_utilization_avg",
                value=resource_stats["cpu"]["avg"],
                unit="percent",
                target=80.0,
                status="pass" if resource_stats["cpu"]["avg"] < 80.0 else "warning",
                timestamp=time.time()
            ),
            PerformanceMetric(
                name="memory_utilization_avg",
                value=resource_stats["memory"]["avg"],
                unit="percent",
                target=75.0,
                status="pass" if resource_stats["memory"]["avg"] < 75.0 else "warning",
                timestamp=time.time()
            )
        ])
        
        # Determine overall status
        overall_status = "pass"
        for metric in metrics:
            if metric.status == "fail":
                overall_status = "fail"
                break
            elif metric.status == "warning" and overall_status == "pass":
                overall_status = "warning"
        
        return BenchmarkResult(
            agent_name=self.agent_name,
            benchmark_type=benchmark_type,
            timestamp=start_time,
            duration=end_time - start_time,
            metrics=metrics,
            overall_status=overall_status,
            system_info=self.system_info
        )
    
    def _execute_benchmark(self, benchmark_type: str) -> List[PerformanceMetric]:
        """Execute specific benchmark - to be overridden by subclasses"""
        raise NotImplementedError("Subclasses must implement _execute_benchmark")

class KernelPerformanceBenchmark(PerformanceBenchmark):
    """Performance benchmarks for kernel-architect agent"""
    
    def _execute_benchmark(self, benchmark_type: str) -> List[PerformanceMetric]:
        metrics = []
        
        if benchmark_type == "memory_management":
            metrics.extend(self._benchmark_memory_management())
        elif benchmark_type == "process_scheduling":
            metrics.extend(self._benchmark_process_scheduling())
        elif benchmark_type == "system_calls":
            metrics.extend(self._benchmark_system_calls())
        elif benchmark_type == "all":
            metrics.extend(self._benchmark_memory_management())
            metrics.extend(self._benchmark_process_scheduling())
            metrics.extend(self._benchmark_system_calls())
        
        return metrics
    
    def _benchmark_memory_management(self) -> List[PerformanceMetric]:
        """Benchmark memory management performance"""
        print("  Benchmarking memory management...")
        
        # Simulate memory allocation/deallocation cycles
        allocation_times = []
        deallocation_times = []
        
        for _ in range(1000):
            # Time memory allocation
            start = time.perf_counter()
            memory_block = bytearray(4096)  # 4KB allocation
            allocation_time = (time.perf_counter() - start) * 1_000_000  # microseconds
            allocation_times.append(allocation_time)
            
            # Time memory deallocation
            start = time.perf_counter()
            del memory_block
            deallocation_time = (time.perf_counter() - start) * 1_000_000  # microseconds
            deallocation_times.append(deallocation_time)
        
        avg_allocation_time = statistics.mean(allocation_times)
        avg_deallocation_time = statistics.mean(deallocation_times)
        
        return [
            PerformanceMetric(
                name="memory_allocation_time",
                value=avg_allocation_time,
                unit="microseconds",
                target=1.0,  # Target: < 1μs
                status="pass" if avg_allocation_time < 1.0 else "fail",
                timestamp=time.time()
            ),
            PerformanceMetric(
                name="memory_deallocation_time",
                value=avg_deallocation_time,
                unit="microseconds",
                target=0.5,  # Target: < 0.5μs
                status="pass" if avg_deallocation_time < 0.5 else "fail",
                timestamp=time.time()
            )
        ]
    
    def _benchmark_process_scheduling(self) -> List[PerformanceMetric]:
        """Benchmark process scheduling performance"""
        print("  Benchmarking process scheduling...")
        
        # Simulate context switch timing
        context_switch_times = []
        
        for _ in range(100):
            start = time.perf_counter()
            # Simulate context switch by creating and switching between threads
            def dummy_work():
                time.sleep(0.001)  # 1ms simulated work
            
            thread = threading.Thread(target=dummy_work)
            thread.start()
            thread.join()
            
            context_switch_time = (time.perf_counter() - start) * 1_000_000  # microseconds
            context_switch_times.append(context_switch_time)
        
        avg_context_switch_time = statistics.mean(context_switch_times)
        
        return [
            PerformanceMetric(
                name="context_switch_time",
                value=avg_context_switch_time,
                unit="microseconds",
                target=10.0,  # Target: < 10μs
                status="pass" if avg_context_switch_time < 10.0 else "fail",
                timestamp=time.time()
            )
        ]
    
    def _benchmark_system_calls(self) -> List[PerformanceMetric]:
        """Benchmark system call performance"""
        print("  Benchmarking system calls...")
        
        # Simulate system call latency
        syscall_times = []
        
        for _ in range(1000):
            start = time.perf_counter()
            os.getpid()  # Simple system call
            syscall_time = (time.perf_counter() - start) * 1_000_000  # microseconds
            syscall_times.append(syscall_time)
        
        avg_syscall_time = statistics.mean(syscall_times)
        
        return [
            PerformanceMetric(
                name="system_call_latency",
                value=avg_syscall_time,
                unit="microseconds",
                target=5.0,  # Target: < 5μs
                status="pass" if avg_syscall_time < 5.0 else "fail",
                timestamp=time.time()
            )
        ]

class GraphicsPerformanceBenchmark(PerformanceBenchmark):
    """Performance benchmarks for gaming-layer-engineer agent"""
    
    def _execute_benchmark(self, benchmark_type: str) -> List[PerformanceMetric]:
        metrics = []
        
        if benchmark_type == "frame_rate":
            metrics.extend(self._benchmark_frame_rate())
        elif benchmark_type == "gpu_utilization":
            metrics.extend(self._benchmark_gpu_utilization())
        elif benchmark_type == "render_latency":
            metrics.extend(self._benchmark_render_latency())
        elif benchmark_type == "all":
            metrics.extend(self._benchmark_frame_rate())
            metrics.extend(self._benchmark_gpu_utilization())
            metrics.extend(self._benchmark_render_latency())
        
        return metrics
    
    def _benchmark_frame_rate(self) -> List[PerformanceMetric]:
        """Benchmark graphics frame rate"""
        print("  Benchmarking frame rate...")
        
        # Simulate frame rendering timing
        frame_times = []
        target_fps = 120
        frame_count = 600  # 5 seconds at 120 FPS
        
        for frame in range(frame_count):
            start = time.perf_counter()
            
            # Simulate frame rendering work
            self._simulate_frame_rendering()
            
            frame_time = time.perf_counter() - start
            frame_times.append(frame_time)
        
        avg_frame_time = statistics.mean(frame_times)
        avg_fps = 1.0 / avg_frame_time if avg_frame_time > 0 else 0
        min_frame_time = min(frame_times)
        max_fps = 1.0 / min_frame_time if min_frame_time > 0 else 0
        
        # Calculate frame drops (frames that took longer than target)
        target_frame_time = 1.0 / target_fps
        dropped_frames = sum(1 for ft in frame_times if ft > target_frame_time)
        drop_percentage = (dropped_frames / frame_count) * 100
        
        return [
            PerformanceMetric(
                name="average_fps",
                value=avg_fps,
                unit="fps",
                target=120.0,
                status="pass" if avg_fps >= 120.0 else "fail",
                timestamp=time.time()
            ),
            PerformanceMetric(
                name="peak_fps",
                value=max_fps,
                unit="fps",
                target=144.0,
                status="pass" if max_fps >= 144.0 else "warning",
                timestamp=time.time()
            ),
            PerformanceMetric(
                name="frame_drop_percentage",
                value=drop_percentage,
                unit="percent",
                target=0.5,  # Target: < 0.5% dropped frames
                status="pass" if drop_percentage < 0.5 else "fail",
                timestamp=time.time()
            )
        ]
    
    def _simulate_frame_rendering(self):
        """Simulate frame rendering work"""
        # Simulate GPU-intensive operations
        data = np.random.rand(1000, 1000)
        np.dot(data, data.T)  # Matrix multiplication to simulate GPU work
    
    def _benchmark_gpu_utilization(self) -> List[PerformanceMetric]:
        """Benchmark GPU utilization efficiency"""
        print("  Benchmarking GPU utilization...")
        
        # Note: In a real implementation, this would interface with GPU monitoring APIs
        # For demonstration, we'll simulate GPU utilization metrics
        
        simulated_gpu_utilization = 72.5  # Simulated GPU utilization percentage
        
        return [
            PerformanceMetric(
                name="gpu_utilization",
                value=simulated_gpu_utilization,
                unit="percent",
                target=80.0,  # Target: < 80% for headroom
                status="pass" if simulated_gpu_utilization < 80.0 else "warning",
                timestamp=time.time()
            )
        ]
    
    def _benchmark_render_latency(self) -> List[PerformanceMetric]:
        """Benchmark rendering pipeline latency"""
        print("  Benchmarking render latency...")
        
        render_latencies = []
        
        for _ in range(100):
            start = time.perf_counter()
            self._simulate_frame_rendering()
            latency = (time.perf_counter() - start) * 1000  # milliseconds
            render_latencies.append(latency)
        
        avg_latency = statistics.mean(render_latencies)
        
        return [
            PerformanceMetric(
                name="render_latency",
                value=avg_latency,
                unit="milliseconds",
                target=8.33,  # Target: < 8.33ms for 120FPS
                status="pass" if avg_latency < 8.33 else "fail",
                timestamp=time.time()
            )
        ]

class AudioPerformanceBenchmark(PerformanceBenchmark):
    """Performance benchmarks for audio-subsystem-engineer agent"""
    
    def _execute_benchmark(self, benchmark_type: str) -> List[PerformanceMetric]:
        metrics = []
        
        if benchmark_type == "audio_latency":
            metrics.extend(self._benchmark_audio_latency())
        elif benchmark_type == "throughput":
            metrics.extend(self._benchmark_audio_throughput())
        elif benchmark_type == "all":
            metrics.extend(self._benchmark_audio_latency())
            metrics.extend(self._benchmark_audio_throughput())
        
        return metrics
    
    def _benchmark_audio_latency(self) -> List[PerformanceMetric]:
        """Benchmark audio system latency"""
        print("  Benchmarking audio latency...")
        
        # Simulate audio buffer processing times
        buffer_process_times = []
        buffer_size = 512  # samples
        sample_rate = 48000  # Hz
        
        for _ in range(1000):
            start = time.perf_counter()
            
            # Simulate audio buffer processing
            audio_data = np.random.rand(buffer_size)
            processed_data = np.fft.fft(audio_data)  # Simulate audio processing
            
            process_time = (time.perf_counter() - start) * 1000  # milliseconds
            buffer_process_times.append(process_time)
        
        avg_process_time = statistics.mean(buffer_process_times)
        theoretical_buffer_time = (buffer_size / sample_rate) * 1000  # milliseconds
        
        return [
            PerformanceMetric(
                name="audio_buffer_process_time",
                value=avg_process_time,
                unit="milliseconds",
                target=theoretical_buffer_time * 0.5,  # Should be much less than buffer time
                status="pass" if avg_process_time < theoretical_buffer_time * 0.5 else "fail",
                timestamp=time.time()
            ),
            PerformanceMetric(
                name="audio_latency",
                value=avg_process_time + theoretical_buffer_time,
                unit="milliseconds",
                target=10.0,  # Target: < 10ms total latency
                status="pass" if (avg_process_time + theoretical_buffer_time) < 10.0 else "fail",
                timestamp=time.time()
            )
        ]
    
    def _benchmark_audio_throughput(self) -> List[PerformanceMetric]:
        """Benchmark audio processing throughput"""
        print("  Benchmarking audio throughput...")
        
        # Test processing throughput
        duration_seconds = 5
        sample_rate = 48000
        channels = 2
        total_samples = duration_seconds * sample_rate * channels
        
        start = time.perf_counter()
        
        # Simulate processing audio stream
        chunk_size = 1024
        processed_samples = 0
        
        while processed_samples < total_samples:
            chunk = np.random.rand(min(chunk_size, total_samples - processed_samples))
            # Simulate audio processing
            processed_chunk = np.fft.fft(chunk)
            processed_samples += len(chunk)
        
        elapsed_time = time.perf_counter() - start
        throughput = total_samples / elapsed_time  # samples per second
        
        return [
            PerformanceMetric(
                name="audio_throughput",
                value=throughput,
                unit="samples_per_second",
                target=sample_rate * channels * 2,  # Should handle 2x real-time easily
                status="pass" if throughput >= sample_rate * channels * 2 else "fail",
                timestamp=time.time()
            )
        ]

class NetworkPerformanceBenchmark(PerformanceBenchmark):
    """Performance benchmarks for network-architect agent"""
    
    def _execute_benchmark(self, benchmark_type: str) -> List[PerformanceMetric]:
        metrics = []
        
        if benchmark_type == "tcp_throughput":
            metrics.extend(self._benchmark_tcp_throughput())
        elif benchmark_type == "latency":
            metrics.extend(self._benchmark_network_latency())
        elif benchmark_type == "all":
            metrics.extend(self._benchmark_tcp_throughput())
            metrics.extend(self._benchmark_network_latency())
        
        return metrics
    
    def _benchmark_tcp_throughput(self) -> List[PerformanceMetric]:
        """Benchmark TCP throughput"""
        print("  Benchmarking TCP throughput...")
        
        # Simulate network data processing
        data_sizes = [1024, 4096, 16384, 65536]  # Different packet sizes
        throughputs = []
        
        for data_size in data_sizes:
            transfer_times = []
            
            for _ in range(100):
                start = time.perf_counter()
                
                # Simulate network data processing
                data = np.random.bytes(data_size)
                processed_data = len(data)  # Simulate processing
                
                transfer_time = time.perf_counter() - start
                transfer_times.append(transfer_time)
            
            avg_transfer_time = statistics.mean(transfer_times)
            throughput = (data_size / avg_transfer_time) / (1024 * 1024)  # MB/s
            throughputs.append(throughput)
        
        avg_throughput = statistics.mean(throughputs)
        
        return [
            PerformanceMetric(
                name="tcp_throughput",
                value=avg_throughput,
                unit="MB/s",
                target=100.0,  # Target: > 100 MB/s for gigabit network
                status="pass" if avg_throughput >= 100.0 else "fail",
                timestamp=time.time()
            )
        ]
    
    def _benchmark_network_latency(self) -> List[PerformanceMetric]:
        """Benchmark network stack latency"""
        print("  Benchmarking network latency...")
        
        # Simulate packet processing latency
        packet_process_times = []
        
        for _ in range(1000):
            start = time.perf_counter()
            
            # Simulate packet processing
            packet_data = np.random.bytes(1500)  # MTU size
            checksum = sum(packet_data) % 256  # Simulate checksum calculation
            
            process_time = (time.perf_counter() - start) * 1_000_000  # microseconds
            packet_process_times.append(process_time)
        
        avg_process_time = statistics.mean(packet_process_times)
        
        return [
            PerformanceMetric(
                name="packet_process_latency",
                value=avg_process_time,
                unit="microseconds",
                target=100.0,  # Target: < 100μs per packet
                status="pass" if avg_process_time < 100.0 else "fail",
                timestamp=time.time()
            )
        ]

class GenericPerformanceBenchmark(PerformanceBenchmark):
    """Generic performance benchmarks for all other agents"""
    
    def _execute_benchmark(self, benchmark_type: str) -> List[PerformanceMetric]:
        metrics = []
        
        if benchmark_type == "cpu_intensive":
            metrics.extend(self._benchmark_cpu_intensive())
        elif benchmark_type == "memory_intensive":
            metrics.extend(self._benchmark_memory_intensive())
        elif benchmark_type == "io_intensive":
            metrics.extend(self._benchmark_io_intensive())
        elif benchmark_type == "all":
            metrics.extend(self._benchmark_cpu_intensive())
            metrics.extend(self._benchmark_memory_intensive())
            metrics.extend(self._benchmark_io_intensive())
        
        return metrics
    
    def _benchmark_cpu_intensive(self) -> List[PerformanceMetric]:
        """Benchmark CPU-intensive operations"""
        print("  Benchmarking CPU-intensive operations...")
        
        start = time.perf_counter()
        
        # CPU-intensive computation
        result = 0
        for i in range(1000000):
            result += i ** 2
        
        elapsed = time.perf_counter() - start
        operations_per_second = 1000000 / elapsed
        
        return [
            PerformanceMetric(
                name="cpu_operations_per_second",
                value=operations_per_second,
                unit="ops/sec",
                target=500000.0,  # Target: > 500K ops/sec
                status="pass" if operations_per_second >= 500000.0 else "warning",
                timestamp=time.time()
            )
        ]
    
    def _benchmark_memory_intensive(self) -> List[PerformanceMetric]:
        """Benchmark memory-intensive operations"""
        print("  Benchmarking memory-intensive operations...")
        
        # Memory allocation and access patterns
        allocation_times = []
        access_times = []
        
        for _ in range(100):
            # Time memory allocation
            start = time.perf_counter()
            large_array = np.random.rand(10000)
            allocation_time = (time.perf_counter() - start) * 1000  # milliseconds
            allocation_times.append(allocation_time)
            
            # Time memory access
            start = time.perf_counter()
            total = np.sum(large_array)
            access_time = (time.perf_counter() - start) * 1000  # milliseconds
            access_times.append(access_time)
        
        avg_allocation_time = statistics.mean(allocation_times)
        avg_access_time = statistics.mean(access_times)
        
        return [
            PerformanceMetric(
                name="memory_allocation_time",
                value=avg_allocation_time,
                unit="milliseconds",
                target=1.0,  # Target: < 1ms for large allocations
                status="pass" if avg_allocation_time < 1.0 else "warning",
                timestamp=time.time()
            ),
            PerformanceMetric(
                name="memory_access_time",
                value=avg_access_time,
                unit="milliseconds",
                target=0.5,  # Target: < 0.5ms for access
                status="pass" if avg_access_time < 0.5 else "warning",
                timestamp=time.time()
            )
        ]
    
    def _benchmark_io_intensive(self) -> List[PerformanceMetric]:
        """Benchmark I/O-intensive operations"""
        print("  Benchmarking I/O-intensive operations...")
        
        # Create temporary files for I/O testing
        import tempfile
        
        write_times = []
        read_times = []
        
        with tempfile.TemporaryDirectory() as temp_dir:
            for i in range(10):
                test_file = Path(temp_dir) / f"test_{i}.dat"
                test_data = np.random.bytes(1024 * 1024)  # 1MB
                
                # Time file write
                start = time.perf_counter()
                with open(test_file, 'wb') as f:
                    f.write(test_data)
                    f.flush()
                    os.fsync(f.fileno())  # Ensure data is written to disk
                write_time = (time.perf_counter() - start) * 1000  # milliseconds
                write_times.append(write_time)
                
                # Time file read
                start = time.perf_counter()
                with open(test_file, 'rb') as f:
                    read_data = f.read()
                read_time = (time.perf_counter() - start) * 1000  # milliseconds
                read_times.append(read_time)
        
        avg_write_time = statistics.mean(write_times)
        avg_read_time = statistics.mean(read_times)
        
        # Calculate throughput (MB/s)
        write_throughput = 1.0 / (avg_write_time / 1000)  # MB/s
        read_throughput = 1.0 / (avg_read_time / 1000)  # MB/s
        
        return [
            PerformanceMetric(
                name="file_write_throughput",
                value=write_throughput,
                unit="MB/s",
                target=50.0,  # Target: > 50 MB/s write
                status="pass" if write_throughput >= 50.0 else "warning",
                timestamp=time.time()
            ),
            PerformanceMetric(
                name="file_read_throughput",
                value=read_throughput,
                unit="MB/s",
                target=100.0,  # Target: > 100 MB/s read
                status="pass" if read_throughput >= 100.0 else "warning",
                timestamp=time.time()
            )
        ]

def create_benchmark(agent_name: str, component_path: str) -> PerformanceBenchmark:
    """Factory function to create appropriate benchmark for agent"""
    
    if agent_name == "kernel-architect":
        return KernelPerformanceBenchmark(agent_name, component_path)
    elif agent_name == "gaming-layer-engineer":
        return GraphicsPerformanceBenchmark(agent_name, component_path)
    elif agent_name == "audio-subsystem-engineer":
        return AudioPerformanceBenchmark(agent_name, component_path)
    elif agent_name == "network-architect":
        return NetworkPerformanceBenchmark(agent_name, component_path)
    else:
        return GenericPerformanceBenchmark(agent_name, component_path)

def main():
    parser = argparse.ArgumentParser(description="RaeenOS Performance Benchmarking Suite")
    parser.add_argument("--agent", required=True, help="Agent name to benchmark")
    parser.add_argument("--component-path", default=".", help="Path to component code")
    parser.add_argument("--benchmark-type", default="all", help="Type of benchmark to run")
    parser.add_argument("--output", help="Output file for results (JSON format)")
    parser.add_argument("--baseline", help="Baseline file for regression detection")
    
    args = parser.parse_args()
    
    # Create and run benchmark
    benchmark = create_benchmark(args.agent, args.component_path)
    result = benchmark.run_benchmark(args.benchmark_type)
    
    # Output results
    result_json = json.dumps(result.to_dict(), indent=2)
    
    if args.output:
        with open(args.output, 'w') as f:
            f.write(result_json)
        print(f"Results written to {args.output}")
    else:
        print("Benchmark Results:")
        print(result_json)
    
    # Check for regressions if baseline provided
    if args.baseline and os.path.exists(args.baseline):
        print("\nChecking for performance regressions...")
        
        with open(args.baseline, 'r') as f:
            baseline_data = json.load(f)
        
        # Compare key metrics
        regression_threshold = 0.05  # 5% regression threshold
        regressions_found = False
        
        for current_metric in result.metrics:
            # Find corresponding baseline metric
            baseline_metric = None
            for baseline_m in baseline_data.get('metrics', []):
                if baseline_m['name'] == current_metric.name:
                    baseline_metric = baseline_m
                    break
            
            if baseline_metric:
                baseline_value = baseline_metric['value']
                current_value = current_metric.value
                
                # Check for regression (higher is worse for latency metrics, lower for throughput)
                if 'latency' in current_metric.name or 'time' in current_metric.name:
                    regression = (current_value - baseline_value) / baseline_value
                else:
                    regression = (baseline_value - current_value) / baseline_value
                
                if regression > regression_threshold:
                    print(f"REGRESSION: {current_metric.name} - {regression*100:.1f}% worse than baseline")
                    regressions_found = True
        
        if not regressions_found:
            print("No significant performance regressions detected.")
    
    # Exit with appropriate code
    if result.overall_status == "fail":
        print(f"\nBenchmark FAILED for {args.agent}")
        sys.exit(1)
    elif result.overall_status == "warning":
        print(f"\nBenchmark completed with WARNINGS for {args.agent}")
        sys.exit(2)
    else:
        print(f"\nBenchmark PASSED for {args.agent}")
        sys.exit(0)

if __name__ == "__main__":
    main()