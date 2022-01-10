#include <atomic>
#include <chrono>
#include <ctime>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "opentelemetry/exporters/ostream/span_exporter.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/trace/provider.h"

namespace trace_api = opentelemetry::trace;
namespace trace_sdk = opentelemetry::sdk::trace;
namespace nostd = opentelemetry::nostd;

namespace {
class Logger {
       public:
	void log(std::string message) {
		auto lk = std::lock_guard(_mutex);
		std::cout << _now() << " [" << std::this_thread::get_id()
			  << "] " << message << std::endl;
	}

       private:
	std::string _now() const {
		time_t rawtime;
		struct tm* timeinfo;
		char buffer[80];

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", timeinfo);
		return buffer;
	}

	std::mutex _mutex;
};

class Worker : public std::enable_shared_from_this<Worker> {
       public:
	explicit Worker(std::shared_ptr<Logger> logger)
	    : _logger(std::move(logger)) {}

	void start() {
		_thread = std::thread(
		    [this, anchor = shared_from_this()] { _run(); });
	}

	void join() {
		_running = false;
		_thread.join();
	}

       private:
	void _run() {
		auto provider = trace_api::Provider::GetTracerProvider();
		auto tracer = provider->GetTracer("My tracer");

		auto rootSpan = tracer->StartSpan("Root");
		_logger->log("Started running");
		while (_running) {
			auto loopSpan = tracer->StartSpan("Loop");
			std::this_thread::sleep_for(kSleepDuration);
		}
		_logger->log("Finished running");
	}

	static constexpr auto kSleepDuration = std::chrono::seconds(1);

	std::shared_ptr<Logger> _logger;

	std::thread _thread;
	std::atomic<bool> _running{true};
};

void initTracer() {
	auto exporter = std::unique_ptr<trace_sdk::SpanExporter>(
	    new opentelemetry::exporter::trace::OStreamSpanExporter);
	auto processor = std::unique_ptr<trace_sdk::SpanProcessor>(
	    new trace_sdk::SimpleSpanProcessor(std::move(exporter)));
	auto provider = nostd::shared_ptr<trace_api::TracerProvider>(
	    new trace_sdk::TracerProvider(std::move(processor)));

	// Set the global trace provider
	trace_api::Provider::SetTracerProvider(provider);
}
}  // namespace

int main() {
	// Setup the tracer
	initTracer();

	const auto kWorkers = 8;
	const auto kTestDuration = std::chrono::seconds(5);

	auto logger = std::make_shared<Logger>();

	std::vector<std::shared_ptr<Worker>> workers(kWorkers);
	for (auto i = 0; i < kWorkers; i++) {
		workers[i] = std::make_shared<Worker>(logger);
		workers[i]->start();
	}

	std::this_thread::sleep_for(kTestDuration);

	for (auto i = 0; i < kWorkers; i++) {
		workers[i]->join();
	}

	return 0;
}
